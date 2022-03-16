#ifndef art_Framework_Principal_ProductRetriever_h
#define art_Framework_Principal_ProductRetriever_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/ProcessTag.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/fwd.h"
#include "canvas/Persistency/Common/WrappedTypeID.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductToken.h"
#include "canvas/Persistency/Provenance/fwd.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/fwd.h"

#include <cassert>
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace art {

  class EDProductGetter;

  namespace detail {
    class Analyzer;
    class Filter;
    class Producer;
  }

  class ProductRetriever {
  public:
    ~ProductRetriever();
    explicit ProductRetriever(BranchType bt,
                              Principal const& p,
                              ModuleContext const& mc,
                              bool recordParents);
    ProductRetriever(ProductRetriever const&) = delete;
    ProductRetriever(ProductRetriever&&) = delete;
    ProductRetriever& operator=(ProductRetriever const&) = delete;
    ProductRetriever& operator=(ProductRetriever&) = delete;

    // Product retrieval
    template <typename PROD>
    PROD const& getProduct(InputTag const& tag) const;
    template <typename PROD>
    PROD const& getProduct(ProductToken<PROD> const& token) const;

    // Product retrieval with provenance access
    template <typename PROD>
    Handle<PROD> getHandle(SelectorBase const&) const;
    template <typename PROD>
    [[deprecated(
      "\n\nart warning: The Event::getHandle<T>(id) interface is deprecated.\n"
      "                 Please use a ProductPtr<T> instead, or retrieve a "
      "parent\n"
      "                 product by using "
      "Ptr<T>::parentAs<Collection>().\n\n")]] Handle<PROD>
    getHandle(ProductID const pid) const;
    template <typename PROD>
    Handle<PROD> getHandle(InputTag const& tag) const;
    template <typename PROD>
    Handle<PROD> getHandle(ProductToken<PROD> const& token) const;

    // Product retrieval with provenance access (guaranteed valid handle)
    template <typename PROD>
    ValidHandle<PROD> getValidHandle(InputTag const& tag) const;
    template <typename PROD>
    ValidHandle<PROD> getValidHandle(ProductToken<PROD> const& token) const;

    // Multiple product retrievals
    template <typename PROD>
    std::vector<InputTag> getInputTags(
      SelectorBase const& selector = MatchAllSelector{}) const;
    template <typename PROD>
    std::vector<ProductToken<PROD>> getProductTokens(
      SelectorBase const& selector = MatchAllSelector{}) const;
    template <typename PROD>
    std::vector<Handle<PROD>> getMany(
      SelectorBase const& selector = MatchAllSelector{}) const;

    // Obsolete product-retrieval (will be deprecated)
    template <typename PROD>
    bool get(SelectorBase const&, Handle<PROD>& result) const;
    template <typename PROD>
    [[deprecated(
      "\n\nart warning: The Event::get(id, handle) interface is deprecated.\n"
      "                 Please use a ProductPtr<T> instead, or retrieve a "
      "parent\n"
      "                 product by using "
      "Ptr<T>::parentAs<Collection>().\n\n")]] bool
    get(ProductID const pid, Handle<PROD>& result) const;
    template <typename PROD>
    bool getByLabel(std::string const& label,
                    std::string const& instance,
                    Handle<PROD>& result) const;
    template <typename PROD>
    bool getByLabel(std::string const& label,
                    std::string const& instance,
                    std::string const& process,
                    Handle<PROD>& result) const;
    template <typename PROD>
    bool getByLabel(InputTag const& tag, Handle<PROD>& result) const;

    // View retrieval
    template <typename ELEMENT>
    std::size_t getView(std::string const& moduleLabel,
                        std::string const& productInstanceName,
                        std::string const& processName,
                        std::vector<ELEMENT const*>& result) const;
    template <typename ELEMENT>
    std::size_t getView(std::string const& moduleLabel,
                        std::string const& productInstanceName,
                        std::vector<ELEMENT const*>& result) const;
    template <typename ELEMENT>
    std::size_t getView(InputTag const&,
                        std::vector<ELEMENT const*>& result) const;
    template <typename ELEMENT>
    std::size_t getView(ViewToken<ELEMENT> const&,
                        std::vector<ELEMENT const*>& result) const;
    template <typename ELEMENT>
    bool getView(std::string const& moduleLabel,
                 std::string const& productInstanceName,
                 std::string const& processName,
                 View<ELEMENT>& result) const;
    template <typename ELEMENT>
    bool getView(std::string const& moduleLabel,
                 std::string const& productInstanceName,
                 View<ELEMENT>& result) const;
    template <typename ELEMENT>
    bool getView(InputTag const&, View<ELEMENT>& result) const;
    template <typename ELEMENT>
    bool getView(ViewToken<ELEMENT> const&, View<ELEMENT>& result) const;

    std::vector<ProductID> retrievedPIDs() const;

    // Miscellaneous functionality
    std::optional<Provenance const> getProductProvenance(ProductID) const;
    std::optional<fhicl::ParameterSet const> getProcessParameterSet(
      std::string const& process) const;
    cet::exempt_ptr<BranchDescription const> getProductDescription(
      ProductID) const;

    EDProductGetter const* productGetter(ProductID const pid) const;
    template <typename T>
    ProductID getProductID(std::string const& instance_name = "") const;

  private:
    void recordAsParent_(cet::exempt_ptr<Group const> grp) const;
    cet::exempt_ptr<Group const> getContainerForView_(
      TypeID const&,
      std::string const& moduleLabel,
      std::string const& productInstanceName,
      ProcessTag const& processTag) const;
    ProductID getProductID_(TypeID const& typeID,
                            std::string const& instance) const;
    std::vector<InputTag> getInputTags_(WrappedTypeID const& wrapped,
                                        SelectorBase const& selector) const;
    GroupQueryResult getByLabel_(WrappedTypeID const& wrapped,
                                 InputTag const& tag) const;
    GroupQueryResult getBySelector_(WrappedTypeID const& wrapped,
                                    SelectorBase const& selector) const;
    GroupQueryResult getByProductID_(ProductID productID) const;
    std::vector<GroupQueryResult> getMany_(WrappedTypeID const& wrapped,
                                           SelectorBase const& sel) const;

    // Protects use of retrievedProducts_ and putProducts_.
    mutable std::recursive_mutex mutex_{};

    // Is this an Event, a Run, a SubRun, or a Results.
    BranchType const branchType_;

    // The principal we are operating on.
    Principal const& principal_;

    // The context of the currently processing module.
    ModuleContext const& mc_;

    // The module we were created for.
    ModuleDescription const& md_;

    // If we are constructed as a non-const Event, then we can be used
    // to put products into the Principal, so we need to record
    // retrieved products into retrievedProducts_ to track parentage
    // of any products we put.
    bool const recordParents_;

    // The set of products retrieved from the principal.  We use this
    // to track parentage of any products we put.
    mutable std::set<ProductID> retrievedProducts_{};
  };

  template <typename PROD>
  ProductID
  ProductRetriever::getProductID(std::string const& instance /* = "" */) const
  {
    return getProductID_(TypeID{typeid(PROD)}, instance);
  }

  // =========================================================================
  template <typename PROD>
  PROD const&
  ProductRetriever::getProduct(InputTag const& tag) const
  {
    return *getValidHandle<PROD>(tag);
  }

  template <typename PROD>
  PROD const&
  ProductRetriever::getProduct(ProductToken<PROD> const& token) const
  {
    return *getValidHandle(token);
  }

  // =========================================================================
  template <typename PROD>
  Handle<PROD>
  ProductRetriever::getHandle(SelectorBase const& sel) const
  {
    auto qr = getBySelector_(WrappedTypeID::make<PROD>(), sel);
    return Handle<PROD>{qr};
  }

  template <typename PROD>
  Handle<PROD>
  ProductRetriever::getHandle(ProductID const pid) const
  {
    auto qr = getByProductID_(pid);
    return Handle<PROD>{qr};
  }

  template <typename PROD>
  Handle<PROD>
  ProductRetriever::getHandle(InputTag const& tag) const
  {
    auto qr = getByLabel_(WrappedTypeID::make<PROD>(), tag);
    return Handle<PROD>{qr};
  }

  template <typename PROD>
  Handle<PROD>
  ProductRetriever::getHandle(ProductToken<PROD> const& token) const
  {
    return getHandle<PROD>(token.inputTag_);
  }

  // =========================================================================
  template <typename PROD>
  ValidHandle<PROD>
  ProductRetriever::getValidHandle(InputTag const& tag) const
  {
    auto h = getHandle<PROD>(tag);
    return ValidHandle{h.product(), h.productGetter(), *h.provenance()};
  }

  template <typename PROD>
  ValidHandle<PROD>
  ProductRetriever::getValidHandle(ProductToken<PROD> const& token) const
  {
    return getValidHandle<PROD>(token.inputTag_);
  }

  template <typename PROD>
  std::vector<InputTag>
  ProductRetriever::getInputTags(SelectorBase const& selector) const
  {
    return getInputTags_(WrappedTypeID::make<PROD>(), selector);
  }

  template <typename PROD>
  std::vector<ProductToken<PROD>>
  ProductRetriever::getProductTokens(SelectorBase const& selector) const
  {
    auto const tags = getInputTags<PROD>(selector);
    std::vector<ProductToken<PROD>> tokens;
    tokens.reserve(tags.size());
    cet::transform_all(tags, back_inserter(tokens), [](auto const& tag) {
      return ProductToken<PROD>{tag};
    });
    return tokens;
  }

  template <typename PROD>
  std::vector<Handle<PROD>>
  ProductRetriever::getMany(SelectorBase const& sel) const
  {
    auto const qrs = getMany_(WrappedTypeID::make<PROD>(), sel);
    std::vector<Handle<PROD>> products;
    products.reserve(qrs.size());
    cet::transform_all(qrs, back_inserter(products), [](auto const& qr) {
      return Handle<PROD>{qr};
    });
    return products;
  }

  template <typename ELEMENT>
  std::size_t
  ProductRetriever::getView(std::string const& moduleLabel,
                            std::string const& productInstanceName,
                            std::string const& processName,
                            std::vector<ELEMENT const*>& result) const
  {
    std::lock_guard lock{mutex_};
    std::size_t const orig_size = result.size();
    auto grp = getContainerForView_(TypeID{typeid(ELEMENT)},
                                    moduleLabel,
                                    productInstanceName,
                                    ProcessTag{processName, md_.processName()});
    if (recordParents_) {
      recordAsParent_(grp);
    }
    auto const view = grp->uniqueProduct()->getView();
    std::vector<ELEMENT const*> castedView;
    for (auto p : view) {
      castedView.push_back(static_cast<ELEMENT const*>(p));
    }
    result = std::move(castedView);
    return result.size() - orig_size;
  }

  template <typename ELEMENT>
  std::size_t
  ProductRetriever::getView(std::string const& moduleLabel,
                            std::string const& productInstanceName,
                            std::vector<ELEMENT const*>& result) const
  {
    return getView(moduleLabel, productInstanceName, {}, result);
  }

  template <typename ELEMENT>
  std::size_t
  ProductRetriever::getView(InputTag const& tag,
                            std::vector<ELEMENT const*>& result) const
  {
    return getView(tag.label(), tag.instance(), tag.process(), result);
  }

  template <typename ELEMENT>
  std::size_t
  ProductRetriever::getView(ViewToken<ELEMENT> const& token,
                            std::vector<ELEMENT const*>& result) const
  {
    return getView(token.inputTag_.label(),
                   token.inputTag_.instance(),
                   token.inputTag_.process(),
                   result);
  }

  template <typename ELEMENT>
  bool
  ProductRetriever::getView(std::string const& moduleLabel,
                            std::string const& productInstanceName,
                            std::string const& processName,
                            View<ELEMENT>& result) const
  {
    std::lock_guard lock{mutex_};
    auto grp = getContainerForView_(TypeID{typeid(ELEMENT)},
                                    moduleLabel,
                                    productInstanceName,
                                    ProcessTag{processName, md_.processName()});
    if (recordParents_) {
      recordAsParent_(grp);
    }
    auto const view = grp->uniqueProduct()->getView();
    std::vector<ELEMENT const*> castedView;
    for (auto p : view) {
      castedView.push_back(static_cast<ELEMENT const*>(p));
    }
    result = View{move(castedView), grp->productID(), grp->uniqueProduct()};
    return true;
  }

  template <typename ELEMENT>
  bool
  ProductRetriever::getView(std::string const& moduleLabel,
                            std::string const& productInstanceName,
                            View<ELEMENT>& result) const
  {
    return getView(moduleLabel, productInstanceName, {}, result);
  }

  template <typename ELEMENT>
  bool
  ProductRetriever::getView(InputTag const& tag, View<ELEMENT>& result) const
  {
    return getView(tag.label(), tag.instance(), tag.process(), result);
  }

  template <typename ELEMENT>
  bool
  ProductRetriever::getView(ViewToken<ELEMENT> const& token,
                            View<ELEMENT>& result) const
  {
    return getView(token.inputTag_.label(),
                   token.inputTag_.instance(),
                   token.inputTag_.process(),
                   result);
  }

  // =======================================================================
  // Obsolete (will be deprecated)
  template <typename PROD>
  bool
  ProductRetriever::get(SelectorBase const& sel, Handle<PROD>& result) const
  {
    result = getHandle<PROD>(sel);
    return static_cast<bool>(result);
  }

  template <typename PROD>
  bool
  ProductRetriever::get(ProductID const pid, Handle<PROD>& result) const
  {
    result = getHandle<PROD>(pid);
    return static_cast<bool>(result);
  }

  template <typename PROD>
  bool
  ProductRetriever::getByLabel(std::string const& moduleLabel,
                               std::string const& productInstanceName,
                               std::string const& processName,
                               Handle<PROD>& result) const
  {
    result = getHandle<PROD>({moduleLabel, productInstanceName, processName});
    return static_cast<bool>(result);
  }

  template <typename PROD>
  bool
  ProductRetriever::getByLabel(std::string const& moduleLabel,
                               std::string const& instance,
                               Handle<PROD>& result) const
  {
    result = getHandle<PROD>({moduleLabel, instance});
    return static_cast<bool>(result);
  }

  template <typename PROD>
  bool
  ProductRetriever::getByLabel(InputTag const& tag, Handle<PROD>& result) const
  {
    result = getHandle<PROD>(tag);
    return static_cast<bool>(result);
  }

  template <typename PROD>
  std::ostream&
  operator<<(std::ostream& os, Handle<PROD> const& h)
  {
    os << h.product() << " " << h.provenance() << " " << h.id();
    return os;
  }

} // namespace art

#endif /* art_Framework_Principal_ProductRetriever_h */

// Local Variables:
// mode: c++
// End:
