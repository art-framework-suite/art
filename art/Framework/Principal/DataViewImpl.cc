#include "art/Framework/Principal/DataViewImpl.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/RangeSetsSupported.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/canonicalProductName.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/fwd.h"
#include "range/v3/view.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace cet;
using namespace std;

namespace art {

  class EDProductGetter;

  DataViewImpl::~DataViewImpl() = default;

  DataViewImpl::DataViewImpl(BranchType const bt,
                             Principal const& principal,
                             ModuleContext const& mc,
                             bool const recordParents)
    : branchType_{bt}
    , principal_{principal}
    , mc_{mc}
    , md_{mc.moduleDescription()}
    , recordParents_{recordParents}
  {}

  EDProductGetter const*
  DataViewImpl::productGetter(ProductID const pid) const
  {
    std::lock_guard lock{mutex_};
    return principal_.productGetter(pid);
  }

  bool
  DataViewImpl::getProcessParameterSet(string const& processName,
                                       fhicl::ParameterSet& ps) const
  {
    std::lock_guard lock{mutex_};
    if (branchType_ != InEvent) {
      return false;
    }
    ProcessHistory ph;
    if (!ProcessHistoryRegistry::get(principal_.processHistoryID(), ph)) {
      throw Exception(errors::NotFound)
        << "ProcessHistoryID " << principal_.processHistoryID()
        << " is not found in the ProcessHistoryRegistry.\n"
        << "This file is malformed.\n";
    }
    auto const config = ph.getConfigurationForProcess(processName);
    if (config) {
      fhicl::ParameterSetRegistry::get(config->parameterSetID(), ps);
    }
    return config.has_value();
  }

  cet::exempt_ptr<BranchDescription const>
  DataViewImpl::getProductDescription(ProductID const pid) const
  {
    return principal_.getProductDescription(pid);
  }

  void
  DataViewImpl::movePutProductsToPrincipal(
    Principal& principal,
    bool const checkProducts,
    map<TypeLabel, BranchDescription> const* expectedProducts)
  {
    assert(branchType_ == InEvent);
    std::lock_guard lock{mutex_};
    if (checkProducts) {
      vector<string> missing;
      for (auto const& [typeLabel, bd] : *expectedProducts) {
        if (putProducts_.find(typeLabel) != putProducts_.cend()) {
          continue;
        }
        ostringstream desc;
        desc << bd;
        missing.emplace_back(desc.str());
      }
      if (!missing.empty()) {
        ostringstream errmsg;
        HorizontalRule rule{25};
        errmsg << "The following products have been declared with 'produces',\n"
               << "but they have not been placed onto the event:\n"
               << rule('=') << '\n';
        for (auto const& desc : missing) {
          errmsg << desc << rule('=') << '\n';
        }
        throw Exception{errors::LogicError, "DataViewImpl::checkPutProducts"}
          << errmsg.str();
      }
    }
    vector<ProductID> const gotPIDs(begin(retrievedProducts_),
                                    end(retrievedProducts_));
    for (auto& pmvalue : putProducts_ | ranges::views::values) {
      auto pp = make_unique<ProductProvenance const>(
        pmvalue.bd_.productID(), productstatus::present(), gotPIDs);
      principal.put(pmvalue.bd_,
                    move(pp),
                    move(pmvalue.prod_),
                    make_unique<RangeSet>(RangeSet::invalid()));
    }
    putProducts_.clear();
  }

  void
  DataViewImpl::movePutProductsToPrincipal(Principal& principal)
  {
    std::lock_guard lock{mutex_};
    for (auto& pmvalue : putProducts_ | ranges::views::values) {
      auto pp = make_unique<ProductProvenance const>(pmvalue.bd_.productID(),
                                                     productstatus::present());
      auto rs = detail::range_sets_supported(branchType_) ?
                  make_unique<RangeSet>(pmvalue.rs_) :
                  make_unique<RangeSet>(RangeSet::invalid());
      principal.put(pmvalue.bd_, move(pp), move(pmvalue.prod_), move(rs));
    }
    putProducts_.clear();
  }

  string const&
  DataViewImpl::getProcessName_(std::string const& specifiedProcessName) const
  {
    return specifiedProcessName == "current_process"s ? md_.processName() :
                                                        specifiedProcessName;
  }

  BranchDescription const&
  DataViewImpl::getProductDescription_(
    TypeID const& type,
    string const& instance,
    bool const alwaysEnableLookupOfProducedProducts /*= false*/) const
  {
    std::lock_guard lock{mutex_};
    auto const product_name = canonicalProductName(
      type.friendlyClassName(), md_.moduleLabel(), instance, md_.processName());
    ProductID const pid{product_name};
    auto bd = principal_.getProductDescription(
      pid, alwaysEnableLookupOfProducedProducts);
    if (!bd || (bd->producedClassName() != type.className())) {
      // Either we did not find the product, or the product we
      // did find does not match (which can happen with Assns
      // since Assns(A,B) and Assns(B,A) have the same ProductID
      // but not the same class name.
      throw Exception(errors::ProductRegistrationFailure,
                      "DataViewImpl::getProductDescription_: error while "
                      "trying to retrieve product description:\n")
        << "No product is registered for\n"
        << "  process name:                '" << md_.processName() << "'\n"
        << "  module label:                '" << md_.moduleLabel() << "'\n"
        << "  product class name:          '" << type.className() << "'\n"
        << "  product friendly class name: '" << type.friendlyClassName()
        << "'\n"
        << "  product instance name:       '" << instance << "'\n"
        << "  branch type:                 '" << branchType_ << "'\n";
    }
    // The description object is owned by either the source or the
    // event processor, whose lifetimes exceed that of the
    // DataViewImpl object.  It is therefore safe to dereference.
    return *bd;
  }

  void
  DataViewImpl::recordAsParent_(exempt_ptr<Group const> grp) const
  {
    if (grp->productDescription().transient()) {
      // If the product retrieved is transient, don't use its
      // ProductID; use the ProductID's of its parents.
      auto const& parents = grp->productProvenance()->parentage().parents();
      retrievedProducts_.insert(cbegin(parents), cend(parents));
    } else {
      retrievedProducts_.insert(grp->productDescription().productID());
    }
  }

  exempt_ptr<Group const>
  DataViewImpl::getContainerForView_(TypeID const& typeID,
                                     string const& moduleLabel,
                                     string const& productInstanceName,
                                     ProcessTag const& processTag) const
  {
    // Check that the consumesView<ELEMENT, BT>(InputTag),
    // or the mayConsumeView<ELEMENT, BT>(InputTag)
    // is actually present.
    ConsumesInfo::instance()->validateConsumedProduct(
      branchType_,
      md_,
      ProductInfo{ProductInfo::ConsumableType::ViewElement,
                  typeID,
                  moduleLabel,
                  productInstanceName,
                  processTag});
    // Fetch the specified data products, which must be containers.
    auto const groups = principal_.getMatchingSequence(
      mc_,
      Selector{ModuleLabelSelector{moduleLabel} &&
               ProductInstanceNameSelector{productInstanceName} &&
               ProcessNameSelector{processTag.name()}},
      processTag);
    auto qrs = resolve_products(groups, TypeID{});
    // Remove any containers that do not allow upcasting of their
    // elements to the desired element type.
    auto new_end =
      remove_if(qrs.begin(), qrs.end(), [&typeID](auto const& gqr) {
        auto const group = gqr.result();
        assert(group->productDescription().supportsView());
        return !detail::upcastAllowed(*group->uniqueProduct()->typeInfo(),
                                      typeID.typeInfo());
      });
    qrs.erase(new_end, qrs.end());
    // Throw if there is not one and only one container to return.
    if (qrs.size() != 1) {
      Exception e{errors::ProductNotFound};
      e << "getView: Found "
        << (qrs.empty() ? "no products" : "more than one product")
        << " matching all criteria\n"
        << "Looking for sequence of type: " << typeID << "\n"
        << "Looking for module label: " << moduleLabel << "\n"
        << "Looking for productInstanceName: " << productInstanceName << "\n";
      if (!processTag.name().empty()) {
        e << "Looking for processName: " << processTag.name() << "\n";
      }
      throw e;
    }
    // And return the single result.
    return qrs[0].result();
  }
} // namespace art
