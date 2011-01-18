#ifndef DataFormats_Common_Wrapper_h
#define DataFormats_Common_Wrapper_h

// ======================================================================
//
// Wrapper: A template wrapper around EDProducts to hold the product ID.
//
// ======================================================================

#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/traits.h"
#include "art/Utilities/Exception.h"
#include "boost/mpl/if.hpp"
#include "cpp0x/type_traits"
#include <deque>
#include <list>
#include <memory>
#include <set>
#include <typeinfo>
#include <vector>

namespace art { namespace detail {
  template< typename T >
    struct has_fillView_member;
} }

// ----------------------------------------------------------------------

namespace art {

  template< class T, bool = detail::has_fillView_member<T>::value >
    struct fillView;

  template <class T>
  class Wrapper
    : public EDProduct
  {
    // non-copyable:
    Wrapper( Wrapper<T> const& );
    void operator = ( Wrapper<T> const& );

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

    virtual void
      fillView( std::vector<void const *> & view ) const
    { art::fillView<T>()(obj, view); }

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

    bool present;
    //   T const obj;
    T obj;

  };  // Wrapper<>

}  // art

namespace art {

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

    template <typename T, void (T::*)(T&)>  struct swap_function;
    template <typename T> no_tag  has_swap_helper(...);
    template <typename T> yes_tag has_swap_helper(swap_function<T, &T::swap> * dummy);

    template<typename T>
    struct has_swap_function
    {
      static bool const value =
        sizeof(has_swap_helper<T>(0)) == sizeof(yes_tag);
    };

    typedef  std::vector<void const *>  vv_t;
    template <typename T, void (T::*)(vv_t&)>  struct fillView_function;
    template <typename T> no_tag  has_fillView_helper(...);
    template <typename T> yes_tag has_fillView_helper(fillView_function<T, &T::fillView> * dummy);

    template< typename T >
    struct has_fillView_member
    {
      static bool const value =
        sizeof(has_fillView_helper<T>(0)) == sizeof(yes_tag);
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

  template< class T >
    struct fillView<T, true>
  {
    void operator () ( T const                   & product
                     , std::vector<void const *> & view
                     )
    {
      product.fillView(view);
    }
  };  // fillView<T>

  template< class T >
    struct fillView<T, false>
  {
    void operator () ( T const                   & product
                     , std::vector<void const *> & view
                     )
    {
      throw Exception(errors::ProductDoesNotSupportViews)
        << "Product type " << typeid(T).name()
        << " has no fillView() capability\n";
    }
  };  // fillView<T>

  template< >
    struct fillView< std::vector<bool>, false >
  {
    void operator () ( std::vector<bool> const   & product
                     , std::vector<void const *> & view
                     )
    {
      throw Exception(errors::ProductDoesNotSupportViews)
        << "Product type std::vector<bool> has no fillView() capability\n";
    }
  };  // fillView<vector<bool>>

  template< class E >
    struct fillView< std::vector<E>, false >
  {
    void operator () ( std::vector<E> const      & product
                     , std::vector<void const *> & view
                     )
    {
      for( typename std::vector<E>::const_iterator b = product.begin()
                                                 , e = product.end()
         ; b != e; ++b )
        view.push_back( &*b );
    }
  };  // fillView<vector<E>>

  template< class E >
    struct fillView< std::list<E>, false >
  {
    void operator () ( std::list<E> const        & product
                     , std::vector<void const *> & view
                     )
    {
      for( typename std::list<E>::const_iterator b = product.begin()
                                               , e = product.end()
         ; b != e; ++b )
        view.push_back( &*b );
    }
  };  // fillView<list<E>>

  template< class E >
    struct fillView< std::deque<E>, false >
  {
    void operator () ( std::deque<E> const       & product
                     , std::vector<void const *> & view
                     )
    {
      for( typename std::deque<E>::const_iterator b = product.begin()
                                                , e = product.end()
         ; b != e; ++b )
        view.push_back( &*b );
    }
  };  // fillView<deque<E>>

  template< class E >
    struct fillView< std::set<E>, false >
  {
    void operator () ( std::set<E> const         & product
                     , std::vector<void const *> & view
                     )
    {
      for( typename std::set<E>::const_iterator b = product.begin()
                                              , e = product.end()
         ; b != e; ++b )
        view.push_back( &*b );
    }
  };  // fillView<set<E>>

}

#endif
