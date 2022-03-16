#include "art/Framework/Principal/ProductRetriever.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ConsumesInfo.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/ProcessTag.h"
#include "art/Framework/Principal/ProductInfo.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
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

  std::optional<fhicl::ParameterSet const>
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

  std::optional<Provenance const>
  ProductRetriever::getProductProvenance(ProductID const pid) const
  {
    auto gqr = principal_.getByProductID(pid);
    if (gqr.failed()) {
      return std::nullopt;
    }

    auto group = gqr.result();
    if (!group->productProvenance()) {
      // This can happen if someone tries to access the provenance
      // before the product has been produced.
      return std::nullopt;
    }
    return std::make_optional<Provenance const>(group);
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

  ProductID
  ProductRetriever::getProductID_(TypeID const& type,
                                  std::string const& instance /* = "" */) const
  {
    std::lock_guard lock{mutex_};
    auto const& product_name = canonicalProductName(
      type.friendlyClassName(), md_.moduleLabel(), instance, md_.processName());
    ProductID const pid{product_name};
    auto desc = principal_.getProductDescription(pid);
    if (!desc) {
      throw Exception(errors::ProductRegistrationFailure,
                      "ProductRetriever::getProductID: error while trying to "
                      "retrieve product description:\n")
        << "No product is registered for\n"
        << "  process name:                '" << md_.processName() << "'\n"
        << "  module label:                '" << md_.moduleLabel() << "'\n"
        << "  product friendly class name: '" << type.friendlyClassName()
        << "'\n"
        << "  product instance name:       '" << instance << "'\n"
        << "  branch type:                 '" << branchType_ << "'\n";
    }
    // The description object is owned by either the source or the
    // event processor, whose lifetimes exceed that of the
    // ProductRetriever object.  It is therefore safe to dereference.
    return desc->productID();
  }

  std::vector<InputTag>
  ProductRetriever::getInputTags_(WrappedTypeID const& wrapped,
                                  SelectorBase const& selector) const
  {
    ProcessTag const processTag{"", md_.processName()};
    return principal_.getInputTags(mc_, wrapped, selector, processTag);
  }

  GroupQueryResult
  ProductRetriever::getByLabel_(WrappedTypeID const& wrapped,
                                InputTag const& tag) const
  {
    std::lock_guard lock{mutex_};
    ProcessTag const processTag{tag.process(), md_.processName()};
    ProductInfo const pinfo{ProductInfo::ConsumableType::Product,
                            wrapped.product_type,
                            tag.label(),
                            tag.instance(),
                            processTag};
    ConsumesInfo::instance()->validateConsumedProduct(branchType_, md_, pinfo);
    GroupQueryResult qr = principal_.getByLabel(
      mc_, wrapped, tag.label(), tag.instance(), processTag);
    bool const ok = qr.succeeded() && !qr.failed();
    if (recordParents_ && ok) {
      recordAsParent_(qr.result());
    }
    return qr;
  }

  GroupQueryResult
  ProductRetriever::getBySelector_(WrappedTypeID const& wrapped,
                                   SelectorBase const& sel) const
  {
    std::lock_guard lock{mutex_};
    // We do *not* track whether consumes was called for a SelectorBase.
    ProcessTag const processTag{"", md_.processName()};
    auto qr = principal_.getBySelector(mc_, wrapped, sel, processTag);
    bool const ok = qr.succeeded() && !qr.failed();
    if (recordParents_ && ok) {
      recordAsParent_(qr.result());
    }
    return qr;
  }

  GroupQueryResult
  ProductRetriever::getByProductID_(ProductID const pid) const
  {
    std::lock_guard lock{mutex_};
    auto qr = principal_.getByProductID(pid);
    bool const ok = qr.succeeded() && !qr.failed();
    if (recordParents_ && ok) {
      recordAsParent_(qr.result());
    }
    return qr;
  }

  std::vector<GroupQueryResult>
  ProductRetriever::getMany_(WrappedTypeID const& wrapped,
                             SelectorBase const& sel) const
  {
    std::lock_guard lock{mutex_};
    ConsumesInfo::instance()->validateConsumedProduct(
      branchType_,
      md_,
      ProductInfo{ProductInfo::ConsumableType::Many, wrapped.product_type});
    ProcessTag const processTag{"", md_.processName()};
    auto qrs = principal_.getMany(mc_, wrapped, sel, processTag);
    for (auto const& qr : qrs) {
      if (recordParents_) {
        recordAsParent_(qr.result());
      }
    }
    return qrs;
  }
} // namespace art
