#ifndef art_Framework_Principal_Consumer_h
#define art_Framework_Principal_Consumer_h

//============================================================================
// Consumer is the base class for all module types that retrieve
// products.  See below for guidance as to which interface should be
// called for a given context.
//
// N.B. All 'consumes*' or 'mayConsume*' calls should be done in a
//      module's constructor, before processing any beginJob
//      callbacks.
//
// Unconditional retrieval of products
// -----------------------------------
//
// For products that are always intended to be retrieved for a given
// module execution, the 'consumes' family of calls should be invoked
// in a user's module constructor (e.g.):
//
//   consumes<int>(input_tag);     // => ProductToken<int>
//   consumesMany<int>();          // => void
//   consumesView<int>(input_tag); // => ViewToken<int>
//
// Making such calls tells the framework that the particular module in
// question will make the following product retrievals (e.g.):
//
//   e.getValidHandle<int>(input_tag);
//   e.getManyByType<int>();
//   art::View<int> v;
//   e.getView(input_tag, v);
//
// The returned type of the consumes calls are shown above.  To
// eventually facilitate faster product lookup, the returned tokens
// can be supplied by the user (e.g.):
//
//   ProductToken<int> intToken_; // data member of module class
//   e.getValidHandle(intToken_); // => ValidHandle<int>
//
//   ViewToken<int> viewToken_; // data member of module class
//   std::vector<int const*> v;
//   e.getView(viewToken_, v);
//
// The consumesMany function template does not return a token.
//
// Conditional retrieval of products
// ---------------------------------
//
// If there are situations in which a product *may* be retrieved
// (e.g. within an 'if' block), then the *mayConsume* interface should
// be considered instead of the consumes interface:
//
//   mayConsume<int>(input_tag); // => ProductToken<int>
//   mayConsumeMany<int>();      // => void
//   mayConsumeView<int>();      // => ViewToken<int>
//
// The return types of the functions are the same as their 'consumes'
// partners for unconditional product retrieving.  However, how the
// tokens are used by the framework may be different than how they are
// used in the 'consumes' case.

// Retrieving products in non-module contexts
// ------------------------------------------
//
// There are cases where products are retrieved in non-module
// contexts.  Although such product retrieval is not forbidden, it is
// not a generally recommended usage pattern.  The 'consumes'
// interface is, therefore, not supported in non-module contexts.
//============================================================================

#include "art/Framework/Principal/ProductInfo.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductToken.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/exempt_ptr.h"

namespace fhicl {
  class ParameterSet;
}

namespace art {

  class DataViewImpl;
  class ModuleDescription;

  class Consumer {
    struct InvalidTag{};
  public:

    explicit Consumer() = default;

    static cet::exempt_ptr<Consumer> non_module_context();

    template <typename T, BranchType = InEvent>
    ProductToken<T> consumes(InputTag const&);

    template <typename T, BranchType = InEvent>
    void consumesMany();

    template <typename Element, BranchType = InEvent>
    ViewToken<Element> consumesView(InputTag const&);

    // mayConsume variants, which should be used whenever product
    // retrievals do not always occur whenever the user is presented
    // with a transactional object.
    template <typename T, BranchType = InEvent>
    ProductToken<T> mayConsume(InputTag const&);

    template <typename T, BranchType = InEvent>
    void mayConsumeMany();

    template <typename Element, BranchType = InEvent>
    ViewToken<Element> mayConsumeView(InputTag const&);

  protected:

    friend class DataViewImpl;
    void validateConsumedProduct(BranchType const bt, ProductInfo const& pi);

    // After the modules are constructed, their ModuleDescription
    // values are assigned.  We receive the values of that assignment.
    void setModuleDescription(ModuleDescription const& md);

    // Once all of the 'consumes(Many)' calls have been made for this
    // recorder, the consumables are sorted, and the configuration is
    // retrieved to specify the desired behavior in the case of a
    // missing consumes clause.
    void prepareForJob(fhicl::ParameterSet const& pset);
    void showMissingConsumes() const;

  private:

    using ConsumableType = ProductInfo::ConsumableType;
    explicit Consumer(InvalidTag) : moduleContext_{false} {}

    bool moduleContext_{true};
    bool requireConsumes_{false};
    ConsumableProducts consumables_{};
    ConsumableProductSets missingConsumes_{};
    cet::exempt_ptr<ModuleDescription const> moduleDescription_{nullptr};
  };
}

//===========================================================================
template <typename T, art::BranchType BT>
art::ProductToken<T>
art::Consumer::consumes(InputTag const& it)
{
  if (!moduleContext_) return ProductToken<T>::invalid();

  consumables_[BT].emplace_back(ConsumableType::Product,
                                TypeID{typeid(T)},
                                it.label(),
                                it.instance(),
                                it.process());
  return ProductToken<T>{it};
}

template <typename T, art::BranchType BT>
void art::Consumer::consumesMany()
{
  if (!moduleContext_) return;

  consumables_[BT].emplace_back(ConsumableType::Many, TypeID{typeid(T)});
}

template <typename T, art::BranchType BT>
art::ViewToken<T>
art::Consumer::consumesView(InputTag const& it)
{
  if (!moduleContext_) return ViewToken<T>::invalid();

  consumables_[BT].emplace_back(ConsumableType::ViewElement,
                                TypeID{typeid(T)},
                                it.label(),
                                it.instance(),
                                it.process());
  return ViewToken<T>{it};
}

//=======================================================================
// mayConsume variants

template <typename T, art::BranchType BT>
art::ProductToken<T>
art::Consumer::mayConsume(InputTag const& it)
{
  if (!moduleContext_) return ProductToken<T>::invalid();

  consumables_[BT].emplace_back(ConsumableType::Product,
                                TypeID{typeid(T)},
                                it.label(),
                                it.instance(),
                                it.process());
  return ProductToken<T>{it};
}

template <typename T, art::BranchType BT>
void art::Consumer::mayConsumeMany()
{
  if (!moduleContext_) return;

  consumables_[BT].emplace_back(ConsumableType::Many, TypeID{typeid(T)});
}

template <typename T, art::BranchType BT>
art::ViewToken<T>
art::Consumer::mayConsumeView(InputTag const& it)
{
  if (!moduleContext_) return ViewToken<T>::invalid();

  consumables_[BT].emplace_back(ConsumableType::ViewElement,
                                TypeID{typeid(T)},
                                it.label(),
                                it.instance(),
                                it.process());
  return ViewToken<T>{it};
}

#endif /* art_Framework_Principal_Consumer_h */

// Local Variables:
// mode: c++
// End:
