#include "art/Framework/Principal/ProductInserter.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/RangeSetsSupported.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/Parentage.h"
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

  ProductInserter::~ProductInserter() = default;

  ProductInserter::ProductInserter(BranchType const bt,
                                   Principal& principal,
                                   ModuleContext const& mc)
    : branchType_{bt}, principal_{&principal}, md_{&mc.moduleDescription()}
  {}

  void
  ProductInserter::commitProducts(
    bool const checkProducts,
    map<TypeLabel, BranchDescription> const* expectedProducts,
    std::vector<ProductID> retrievedPIDs)
  {
    assert(branchType_ == InEvent);
    std::lock_guard lock{*mutex_};
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
        throw Exception{errors::LogicError, "ProductInserter::checkPutProducts"}
          << errmsg.str();
      }
    }

    for (auto&& [product, pd, rs] : putProducts_ | ranges::views::values) {
      auto pp = make_unique<ProductProvenance const>(
        pd.productID(), productstatus::present(), retrievedPIDs);
      principal_->put(pd,
                      move(pp),
                      move(product),
                      make_unique<RangeSet>(RangeSet::invalid()));
    }
    putProducts_.clear();
  }

  void
  ProductInserter::commitProducts()
  {
    std::lock_guard lock{*mutex_};
    for (auto&& [product, pd, range_set] :
         putProducts_ | ranges::views::values) {
      auto pp = make_unique<ProductProvenance const>(pd.productID(),
                                                     productstatus::present());
      auto rs = detail::range_sets_supported(branchType_) ?
                  make_unique<RangeSet>(std::move(range_set)) :
                  make_unique<RangeSet>(RangeSet::invalid());
      principal_->put(pd, move(pp), move(product), move(rs));
    }
    putProducts_.clear();
  }

  BranchDescription const&
  ProductInserter::getProductDescription_(
    TypeID const& type,
    string const& instance,
    bool const alwaysEnableLookupOfProducedProducts /*= false*/) const
  {
    std::lock_guard lock{*mutex_};
    auto const product_name = canonicalProductName(type.friendlyClassName(),
                                                   md_->moduleLabel(),
                                                   instance,
                                                   md_->processName());
    ProductID const pid{product_name};
    auto bd = principal_->getProductDescription(
      pid, alwaysEnableLookupOfProducedProducts);
    if (!bd || (bd->producedClassName() != type.className())) {
      // Either we did not find the product, or the product we
      // did find does not match (which can happen with Assns
      // since Assns(A,B) and Assns(B,A) have the same ProductID
      // but not the same class name.
      throw Exception(errors::ProductRegistrationFailure,
                      "ProductInserter::getProductDescription_: error while "
                      "trying to retrieve product description:\n")
        << "No product is registered for\n"
        << "  process name:                '" << md_->processName() << "'\n"
        << "  module label:                '" << md_->moduleLabel() << "'\n"
        << "  product class name:          '" << type.className() << "'\n"
        << "  product friendly class name: '" << type.friendlyClassName()
        << "'\n"
        << "  product instance name:       '" << instance << "'\n"
        << "  branch type:                 '" << branchType_ << "'\n";
    }
    // The description object is owned by either the source or the
    // event processor, whose lifetimes exceed that of the
    // ProductInserter object.  It is therefore safe to dereference.
    return *bd;
  }

  EDProductGetter const*
  ProductInserter::productGetter_(ProductID const id) const
  {
    return principal_->productGetter(id);
  }

  Provenance
  ProductInserter::provenance_(ProductID const id) const
  {
    return principal_->provenance(id);
  }

} // namespace art
