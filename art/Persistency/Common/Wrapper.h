#ifndef DataFormats_Common_Wrapper_h
#define DataFormats_Common_Wrapper_h

/*----------------------------------------------------------------------

Wrapper: A template wrapper around EDProducts to hold the product ID.

----------------------------------------------------------------------*/

#include <algorithm>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>
#include <list>
#include <deque>
#include <set>

#include "boost/mpl/if.hpp"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/traits.h"
#include "art/Utilities/EDMException.h"

namespace edm {

  template <class T>
  class Wrapper : public EDProduct {
  public:
    typedef T value_type;
    typedef T wrapped_type;  // used with Reflex to identify Wrappers
    Wrapper() : EDProduct(), present(false), obj() {}
    explicit Wrapper(std::auto_ptr<T> ptr);
    virtual ~Wrapper() {}
    T const * product() const {return (present ? &obj : 0);}
    T const * operator->() const {return product();}

    //these are used by FWLite
    static const std::type_info& productTypeInfo() { return typeid(T);}
    static const std::type_info& typeInfo() { return typeid(Wrapper<T>);}

    /**REFLEX must call the following constructor
        the constructor takes ownership of T* */
    Wrapper(T*);

  private:
    virtual bool isPresent_() const {return present;}
#ifndef __REFLEX__
    virtual bool isMergeable_() const;
    virtual bool mergeProduct_(EDProduct const* newProduct);
    virtual bool hasIsProductEqual_() const;
    virtual bool isProductEqual_(EDProduct const* newProduct) const;
#endif

    // We wish to disallow copy construction and assignment.
    // We make the copy constructor and assignment operator private.
    Wrapper(Wrapper<T> const& rh); // disallow copy construction
    Wrapper<T> & operator=(Wrapper<T> const&); // disallow assignment

    bool present;
    //   T const obj;
    T obj;
  };

} //namespace edm

namespace edm {

  // This is an attempt to optimize for speed, by avoiding the copying
  // of large objects of type T. In this initial version, we assume
  // that for any class having a 'swap' member function should call
  // 'swap' rather than copying the object.

  template <typename T>
  struct DoSwap
  {
    void operator()(T& a, T& b) { a.swap(b); }
  };

  template <typename T>
  struct DoAssign
  {
    void operator()(T& a, T& b) { a = b; }
  };

#ifndef __REFLEX__
  template <typename T>
  struct IsMergeable
  {
    bool operator()(T const& a) const { return true; }
  };

  template <typename T>
  struct IsNotMergeable
  {
    bool operator()(T const& a) const { return false; }
  };

  template <typename T>
  struct DoMergeProduct
  {
    bool operator()(T & a, T const& b) { return a.mergeProduct(b); }
  };

  template <typename T>
  struct DoNotMergeProduct
  {
    bool operator()(T & a, T const& b) { return true; }
  };

  template <typename T>
  struct DoHasIsProductEqual
  {
    bool operator()(T const& a) const { return true; }
  };

  template <typename T>
  struct DoNotHasIsProductEqual
  {
    bool operator()(T const& a) const { return false; }
  };

  template <typename T>
  struct DoIsProductEqual
  {
    bool operator()(T const& a, T const& b) const { return a.isProductEqual(b); }
  };

  template <typename T>
  struct DoNotIsProductEqual
  {
    bool operator()(T const& a, T const& b) const { return true; }
  };
#endif

  //------------------------------------------------------------
  // Metafunction support for compile-time selection of code used in
  // Wrapper constructor
  //

  namespace detail
  {
    typedef char (& no_tag)[1]; // type indicating FALSE
    typedef char (& yes_tag)[2]; // type indicating TRUE

    // Definitions for the following struct and function templates are
    // not needed; we only require the declarations.
    template <typename T, void (T::*)(T&)>  struct swap_function;
    template <typename T> no_tag  has_swap_helper(...);
    template <typename T> yes_tag has_swap_helper(swap_function<T, &T::swap> * dummy);

    template<typename T>
    struct has_swap_function
    {
      static bool const value =
        sizeof(has_swap_helper<T>(0)) == sizeof(yes_tag);
    };

#ifndef __REFLEX__
    template <typename T, bool (T::*)(T const &)>  struct mergeProduct_function;
    template <typename T> no_tag  has_mergeProduct_helper(...);
    template <typename T> yes_tag has_mergeProduct_helper(mergeProduct_function<T, &T::mergeProduct> * dummy);

    template<typename T>
    struct has_mergeProduct_function
    {
      static bool const value =
        sizeof(has_mergeProduct_helper<T>(0)) == sizeof(yes_tag);
    };

    template <typename T, bool (T::*)(T const &)>  struct isProductEqual_function;
    template <typename T> no_tag  has_isProductEqual_helper(...);
    template <typename T> yes_tag has_isProductEqual_helper(isProductEqual_function<T, &T::isProductEqual> * dummy);

    template<typename T>
    struct has_isProductEqual_function
    {
      static bool const value =
        sizeof(has_isProductEqual_helper<T>(0)) == sizeof(yes_tag);
    };
#endif
  }

  template <class T>
  Wrapper<T>::Wrapper(std::auto_ptr<T> ptr) :
    EDProduct(),
    present(ptr.get() != 0),
    obj()
  {
    if (present) {
      // The following will call swap if T has such a function,
      // and use assignment if T has no such function.
      typename boost::mpl::if_c<detail::has_swap_function<T>::value,
        DoSwap<T>,
        DoAssign<T> >::type swap_or_assign;
      swap_or_assign(obj, *ptr);
    }
  }

  template <class T>
  Wrapper<T>::Wrapper(T* ptr) :
  EDProduct(),
  present(ptr != 0),
  obj()
  {
     std::auto_ptr<T> temp(ptr);
     if (present) {
        // The following will call swap if T has such a function,
        // and use assignment if T has no such function.
        typename boost::mpl::if_c<detail::has_swap_function<T>::value,
         DoSwap<T>,
        DoAssign<T> >::type swap_or_assign;
        swap_or_assign(obj, *ptr);
     }

  }

#ifndef __REFLEX__
  template <class T>
  bool Wrapper<T>::isMergeable_() const
  {
    typename boost::mpl::if_c<detail::has_mergeProduct_function<T>::value,
      IsMergeable<T>,
      IsNotMergeable<T> >::type is_mergeable;
    return is_mergeable(obj);
  }

  template <class T>
  bool Wrapper<T>::mergeProduct_(EDProduct const* newProduct)
  {
    Wrapper<T> const* wrappedNewProduct = dynamic_cast<Wrapper<T> const* >(newProduct);
    if (wrappedNewProduct == 0) return false;
    typename boost::mpl::if_c<detail::has_mergeProduct_function<T>::value,
      DoMergeProduct<T>,
      DoNotMergeProduct<T> >::type merge_product;
    return merge_product(obj, wrappedNewProduct->obj);
  }

  template <class T>
  bool Wrapper<T>::hasIsProductEqual_() const
  {
    typename boost::mpl::if_c<detail::has_isProductEqual_function<T>::value,
      DoHasIsProductEqual<T>,
      DoNotHasIsProductEqual<T> >::type has_is_equal;
    return has_is_equal(obj);
  }

  template <class T>
  bool Wrapper<T>::isProductEqual_(EDProduct const* newProduct) const
  {
    Wrapper<T> const* wrappedNewProduct = dynamic_cast<Wrapper<T> const* >(newProduct);
    if (wrappedNewProduct == 0) return false;
    typename boost::mpl::if_c<detail::has_isProductEqual_function<T>::value,
      DoIsProductEqual<T>,
      DoNotIsProductEqual<T> >::type is_equal;
    return is_equal(obj, wrappedNewProduct->obj);
  }
#endif
}

#endif
