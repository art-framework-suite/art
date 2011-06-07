#ifndef art_Persistency_Common_Ptr_h
#define art_Persistency_Common_Ptr_h

// ======================================================================
//
// Ptr: Persistent 'pointer' to an item in a collection where the
//      collection is in the art::Event
//
// ======================================================================

#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Common/GetProduct.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/OrphanHandle.h"
#include "art/Persistency/Common/RefCore.h"
#include "art/Persistency/Common/traits.h"
#include "art/Utilities/Exception.h"
#include "cpp0x/type_traits"
#include <cstddef>
#include <list>
#include <vector>

namespace art
{
  template <typename T>
  class Ptr
  {
    friend class PtrVectorBase;
  public:

    typedef std::size_t   key_type;
    typedef T             value_type;

    // Create a Ptr<T> to a specific element within a collection of type
    // C. The collection is identified by 'handle', and the element in
    // the collection is identified by the index 'idx'.
    template <typename C>
    Ptr(Handle<C> const& handle, key_type idx, bool setNow=true);

    // An OrphanHandle is a handle to a collection that has been put
    // into an Event during the running of the same module.  This
    // constructor creates a Ptr<T> to a specific element within such a
    // collection of type C. The collection is identified by 'handle',
    // and the element in the collection is identified by index 'idx'.
    template <typename C>
    Ptr(OrphanHandle<C> const& handle, key_type idx, bool setNow=true);

    // Constructor for those users who do not have a product handle,
    // but have a pointer to a product getter (such as the EventPrincipal).
    // prodGetter will ususally be a pointer to the event principal.
    Ptr(ProductID const& productID, key_type itemKey, EDProductGetter const* prodGetter) :
      core_(productID, 0, mustBeNonZero(prodGetter, "Ptr", productID)), key_(itemKey)
    { }

    // Constructor for use in the various X::fillView(...) functions
    // or for extracting a persistent Ptr from a PtrVector.
    // It is an error (not diagnosable at compile- or run-time) to call
    // this constructor with a pointer to a T unless the pointed-to T
    // object is already in a collection of type C stored in the
    // Event. The given ProductID must be the id of the collection
    // in the Event.
    Ptr(ProductID const& productID, T const* item, key_type item_key) :
      core_(productID, item, 0),
      key_(item_key)
    { }

    // Constructor that creates an invalid ("null") Ptr that is
    // associated with a given product (denoted by that product's ProductID).
    explicit Ptr(ProductID const& id) :
      core_(id, 0, 0),
      key_(key_traits<key_type>::value)
    { }

    Ptr():
      core_(),
      key_(key_traits<key_type>::value)
    { }

    template<typename U>
    Ptr(Ptr<U> const& iOther, typename std::enable_if<std::is_base_of<T, U>::value>::type * = 0):
      core_(iOther.id(),
            (iOther.hasCache()? static_cast<T const*>(iOther.get()): static_cast<T const*>(0)),
            iOther.productGetter()),
      key_(iOther.key())
    { }

    template<typename U>
    explicit
    Ptr(Ptr<U> const& iOther, typename std::enable_if<std::is_base_of<U, T>::value>::type * = 0):
      core_(iOther.id(),
            dynamic_cast<T const*>(iOther.get()),
            0),
      key_(iOther.key())
    { }

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    // Dereference operator
    T const&
    operator*() const;

    // Member dereference operator
    T const*
    operator->() const;

    // Returns C++ pointer to the item
    T const* get() const
    {
      return isNull() ? 0 : this->operator->();
    }

    // Checks for null
    bool isNull() const { return !isNonnull(); }

    // Checks for non-null
    bool isNonnull() const { return key_traits<key_type>::value != key_; }
    // Checks for null
    //    bool operator!() const { return isNull(); }

    // Checks if collection is in memory or available
    // in the event. No type checking is done.
    bool isAvailable() const { return core_.isAvailable(); }

    // Accessor for product ID.
    ProductID id() const { return core_.id(); }

    // Accessor for product getter.
    EDProductGetter const* productGetter() const { return core_.productGetter(); }

    key_type key() const { return key_; }

    bool hasCache() const { return 0!=core_.productPtr(); }

    RefCore const& refCore() const { return core_; }

    // Fulfills the role of, "convertible to bool"
    operator void const *() const { return get(); }

    // MUST UPDATE WHEN CLASS IS CHANGED!
    static short Class_Version() { return 10; }

  private:
    template<typename C>
    T const* getItem_(C const* product, key_type iKey);

    void getData_() const {
      if(!hasCache() && 0 != productGetter()) {
        void const* ad = 0;
        const EDProduct* prod = productGetter()->getIt(core_.id());
        if(prod==0) {
          throw art::Exception(errors::ProductNotFound)
            << "A request to resolve an art::Ptr to a product containing items of type: "
            << typeid(T).name()
            << " with ProductID "<<core_.id()
            << "\ncan not be satisfied because the product cannot be found."
            << "\nProbably the branch containing the product is not stored in the input file.\n";
        }
        prod->setPtr(typeid(T),
                     key_,
                     ad);
        core_.setProductPtr(ad);
      }
    }
    // ---------- member data --------------------------------
    RefCore core_;
    key_type key_;
  };  // Ptr<>

  //------------------------------------------------------------
  // Implementation details below.
  //------------------------------------------------------------

  namespace detail {
    template <typename T, typename C>
    class ItemGetter {
    public:
      T const *operator()(C const *product,
                          typename Ptr<T>::key_type iKey) const;
    };

    template <typename T, typename C>
    inline
    T const *
    ItemGetter<T, C>::operator()(C const *product,
                                 typename Ptr<T>::key_type iKey) const {
      assert (product != 0);
      typename C::const_iterator it = product->begin();
      advance(it,iKey);
      T const* address = detail::GetProduct<C>::address(it);
      return address;
    }

    template <typename T>
    class ItemGetter<T, cet::map_vector<T> > {
    public:
      T const *operator()(cet::map_vector<T> const *product,
                          typename Ptr<T>::key_type iKey) const;
    };

    template <typename T>
    inline
    T const *ItemGetter<T, cet::map_vector<T> >::
    operator()(cet::map_vector<T> const *product,
               typename Ptr<T>::key_type iKey) const {
      assert (product != 0);
      cet::map_vector_key k(iKey);
      return product->getOrNull(k);
    }

    template <typename T>
    class ItemGetter<std::pair<cet::map_vector_key, T>, cet::map_vector<T> > {
    public:
      std::pair<cet::map_vector_key, T> const *
      operator()(cet::map_vector<T> const *product,
                 typename Ptr<T>::key_type iKey) const;
    };

    template <typename T>
    inline
    std::pair<cet::map_vector_key, T> const *
    ItemGetter<std::pair<cet::map_vector_key, T>, cet::map_vector<T> >::
    operator()(cet::map_vector<T> const *product,
               typename Ptr<T>::key_type iKey) const {
      assert (product != 0);
      cet::map_vector_key k(static_cast<unsigned>(iKey));
      typename cet::map_vector<T>::const_iterator it = product->find(k);
      if (it == product->end()) {
        return nullptr;
      } else {
        return &(*it);
      }
    }
  }  // namespace detail

  template <typename T>
  template <typename C>
  inline
  Ptr<T>::Ptr(Handle<C> const& handle,
              typename Ptr<T>::key_type idx,
              bool setNow) :
    core_(handle.id(), getItem_(handle.product(), idx), 0),
    key_(idx)
  { }

  template <typename T>
  template <typename C>
  inline
  Ptr<T>::Ptr(OrphanHandle<C> const& handle,
              key_type idx,
              bool setNow) :
    core_(handle.id(), getItem_(handle.product(), idx), 0),
    key_(idx)
  { }

  template<typename T>
  template<typename C>
  inline
  T const*
  Ptr<T>::getItem_(C const* product, key_type iKey)
  {
    return detail::ItemGetter<T, C>()(product, iKey);
  }

  // Dereference operator
  template <typename T>
  inline
  T const&
  Ptr<T>::operator*() const {
    getData_();
    return *reinterpret_cast<T const*>(core_.productPtr());
  }

  // Member dereference operator
  template <typename T>
  inline
  T const*
  Ptr<T>::operator->() const {
    getData_();
    return reinterpret_cast<T const*>(core_.productPtr());
  }

  template <typename T>
  inline
  bool
  operator==(Ptr<T> const& lhs, Ptr<T> const& rhs) {
    return lhs.refCore() == rhs.refCore() && lhs.key() == rhs.key();
  }

  template <typename T>
  inline
  bool
  operator<(Ptr<T> const& lhs, Ptr<T> const& rhs) {
    // The ordering of integer keys guarantees that the ordering of Ptrs within
    // a collection will be identical to the ordering of the referenced objects in the collection.
    return (lhs.refCore() == rhs.refCore() ? lhs.key() < rhs.key() : lhs.refCore() < rhs.refCore());
  }

  template <class T, class C>
  void
  fill_ptr_vector(std::vector<Ptr<T> >& ptrs, Handle<C> const& h)
  {
    for (std::size_t i = 0, sz = h->size(); i != sz; ++i)
      ptrs.push_back(Ptr<T>(h, i));
  }

  template <class T, class C>
  void
  fill_ptr_list(std::list<Ptr<T> >& ptrs, Handle<C> const& h)
  {
    for (std::size_t i = 0, sz = h->size(); i != sz; ++i)
      ptrs.push_back(Ptr<T>(h, i));
  }

}  // art

#endif /* art_Persistency_Common_Ptr_h */

// Local Variables:
// mode: c++
// End:
