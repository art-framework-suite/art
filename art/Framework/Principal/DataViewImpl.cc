#include "art/Framework/Principal/DataViewImpl.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/ProductSemantics.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/canonicalProductName.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "hep_concurrency/RecursiveMutex.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
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
                             bool const recordParents,
                             RangeSet const& rs /* = RangeSet::invalid() */)
    : branchType_{bt}
    , principal_{principal}
    , mc_{mc}
    , md_{mc.moduleDescription()}
    , recordParents_{recordParents}
    , rangeSet_{rs}
  {}

  RunID
  DataViewImpl::runID() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.runID();
  }

  SubRunID
  DataViewImpl::subRunID() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.subRunID();
  }

  EventID
  DataViewImpl::eventID() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.eventID();
  }

  RunNumber_t
  DataViewImpl::run() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.run();
  }

  SubRunNumber_t
  DataViewImpl::subRun() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.subRun();
  }

  EventNumber_t
  DataViewImpl::event() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.event();
  }

  Timestamp const&
  DataViewImpl::beginTime() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.beginTime();
  }

  Timestamp const&
  DataViewImpl::endTime() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.endTime();
  }

  Timestamp
  DataViewImpl::time() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.time();
  }

  bool
  DataViewImpl::isRealData() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.isReal();
  }

  EventAuxiliary::ExperimentType
  DataViewImpl::experimentType() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.ExperimentType();
  }

  History const&
  DataViewImpl::history() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.history();
  }

  ProcessHistoryID const&
  DataViewImpl::processHistoryID() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.history().processHistoryID();
  }

  ProcessHistory const&
  DataViewImpl::processHistory() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.processHistory();
  }

  EDProductGetter const*
  DataViewImpl::productGetter(ProductID const pid) const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return principal_.productGetter(pid);
  }

  bool
  DataViewImpl::getProcessParameterSet(string const& processName,
                                       fhicl::ParameterSet& ps) const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    if (branchType_ != InEvent) {
      return false;
    }
    ProcessHistory ph;
    if (!ProcessHistoryRegistry::get(principal_.history().processHistoryID(),
                                     ph)) {
      throw Exception(errors::NotFound)
        << "ProcessHistoryID " << principal_.history().processHistoryID()
        << " is not found in the ProcessHistoryRegistry.\n"
        << "This file is malformed.\n";
    }
    ProcessConfiguration config;
    bool const process_found =
      ph.getConfigurationForProcess(processName, config);
    if (process_found) {
      fhicl::ParameterSetRegistry::get(config.parameterSetID(), ps);
    }
    return process_found;
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
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    if (checkProducts) {
      vector<string> missing;
      for (auto const& typeLabel_and_bd : *expectedProducts) {
        if (putProducts_.find(typeLabel_and_bd.first) != putProducts_.cend()) {
          continue;
        }
        ostringstream desc;
        desc << typeLabel_and_bd.second;
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
    for (auto& type_label_and_pmvalue : putProducts_) {
      auto& pmvalue = type_label_and_pmvalue.second;
      unique_ptr<ProductProvenance const> pp;
      if (branchType_ == InEvent) {
        vector<ProductID> gotPIDs;
        if (!retrievedProducts_.empty()) {
          gotPIDs.reserve(retrievedProducts_.size());
          gotPIDs.assign(retrievedProducts_.begin(), retrievedProducts_.end());
        }
        pp = make_unique<ProductProvenance const>(
          pmvalue.bd_.productID(), productstatus::present(), gotPIDs);
      } else {
        pp = make_unique<ProductProvenance const>(pmvalue.bd_.productID(),
                                                  productstatus::present());
      }
      if ((branchType_ == InRun) || (branchType_ == InSubRun)) {
        principal.put(pmvalue.bd_,
                      move(pp),
                      move(pmvalue.prod_),
                      make_unique<RangeSet>(pmvalue.rs_));
      } else {
        principal.put(
          pmvalue.bd_, move(pp), move(pmvalue.prod_), make_unique<RangeSet>());
      }
    };
    putProducts_.clear();
  }

  void
  DataViewImpl::movePutProductsToPrincipal(Principal& principal)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& type_label_and_pmvalue : putProducts_) {
      auto& pmvalue = type_label_and_pmvalue.second;
      unique_ptr<ProductProvenance const> pp =
        make_unique<ProductProvenance const>(pmvalue.bd_.productID(),
                                             productstatus::present());
      if ((branchType_ == InRun) || (branchType_ == InSubRun)) {
        principal.put(pmvalue.bd_,
                      move(pp),
                      move(pmvalue.prod_),
                      make_unique<RangeSet>(pmvalue.rs_));
      } else {
        principal.put(
          pmvalue.bd_, move(pp), move(pmvalue.prod_), make_unique<RangeSet>());
      }
    };
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
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    auto const& product_name = canonicalProductName(
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
    auto qrs = principal_.getMatchingSequence(
      mc_,
      Selector{ModuleLabelSelector{moduleLabel} &&
               ProductInstanceNameSelector{productInstanceName} &&
               ProcessNameSelector{processTag.name()}},
      processTag);
    // Remove any containers that do not allow upcasting of their
    // elements to the desired element type.
    qrs.erase(remove_if(qrs.begin(),
                        qrs.end(),
                        [&typeID](auto const& qr) {
                          assert(
                            qr.result()->productDescription().supportsView());
                          return !detail::upcastAllowed(
                            *qr.result()->uniqueProduct()->typeInfo(),
                            typeID.typeInfo());
                        }),
              qrs.end());
    // Throw if there is not one and only one container to return.
    if (qrs.size() != 1) {
      Exception e{errors::ProductNotFound};
      e << "getView: Found "
        << (qrs.size() == 0 ? "no products" : "more than one product")
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
