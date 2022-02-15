#include "art/Framework/Principal/ProductRetriever.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/fwd.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace std::string_literals;

namespace art {

  ProductRetriever::~ProductRetriever() = default;

  ProductRetriever::ProductRetriever(BranchType const bt,
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
  ProductRetriever::productGetter(ProductID const pid) const
  {
    std::lock_guard lock{mutex_};
    return principal_.productGetter(pid);
  }

  std::optional<fhicl::ParameterSet>
  ProductRetriever::getProcessParameterSet(std::string const& processName) const
  {
    std::lock_guard lock{mutex_};
    auto const config =
      principal_.processHistory().getConfigurationForProcess(processName);
    if (!config) {
      return std::nullopt;
    }

    if (fhicl::ParameterSet ps;
        fhicl::ParameterSetRegistry::get(config->parameterSetID(), ps)) {
      return std::make_optional(std::move(ps));
    }
    return std::nullopt;
  }

  std::vector<ProductID>
  ProductRetriever::retrievedPIDs() const
  {
    std::lock_guard lock{mutex_};
    return std::vector<ProductID>(begin(retrievedProducts_),
                                  end(retrievedProducts_));
  }

  cet::exempt_ptr<BranchDescription const>
  ProductRetriever::getProductDescription(ProductID const pid) const
  {
    return principal_.getProductDescription(pid);
  }

  void
  ProductRetriever::recordAsParent_(cet::exempt_ptr<Group const> grp) const
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

  cet::exempt_ptr<Group const>
  ProductRetriever::getContainerForView_(TypeID const& typeID,
                                         std::string const& moduleLabel,
                                         std::string const& productInstanceName,
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
