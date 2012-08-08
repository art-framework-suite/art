#ifndef art_Persistency_Common_Wrapper_h
#define art_Persistency_Common_Wrapper_h

// ======================================================================
//
// Wrapper: A template wrapper around EDProducts to hold the product ID.
//
// ======================================================================

#include "Reflex/Object.h"
#include "Reflex/Type.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/GetProduct.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Common/traits.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/detail/metaprogramming.h"
#include "boost/lexical_cast.hpp"
#include "cetlib/demangle.h"
#include "cetlib/map_vector.h"
#include "cpp0x/memory"
#include "cpp0x/type_traits"

#include <deque>
#include <list>
#include <set>
#include <string>
#include <typeinfo>
#include <vector>

// Required for specializations of has_size_member<T>, below.
namespace CLHEP {
  class HepMatrix;
  class HepSymMatrix;
}

namespace art {
  namespace detail {
    template< typename T >
    struct has_fillView_member;
    template< typename T >
    struct has_size_member;
    template<>
    struct has_size_member<CLHEP::HepMatrix>;
    template<>
    struct has_size_member<CLHEP::HepSymMatrix>;
    template <typename T>
    struct has_makePartner_member;
  }
}

// ----------------------------------------------------------------------

namespace art {

  template< class T, bool = detail::has_fillView_member<T>::value >
  struct fillView;

  template< class T, bool = detail::has_size_member<T>::value >
  struct productSize;

  template <class T>
  class Wrapper
      : public EDProduct {
  public:
    typedef T value_type;
    typedef T wrapped_type;  // used with Reflex to identify Wrappers
    Wrapper() : EDProduct(), present(false), obj() {}
    explicit Wrapper(std::unique_ptr<T> && ptr);
    virtual ~Wrapper() {}
    T const * product() const {return (present ? &obj : 0);}
    T const * operator->() const {return product();}

    //these are used by FWLite
    static const std::type_info & productTypeInfo() { return typeid(T);}
    static const std::type_info & typeInfo() { return typeid(Wrapper<T>);}

    virtual void
    fillView(std::vector<void const *> & view) const
    { art::fillView<T>()(obj, view); }

    virtual std::string
    productSize() const
    { return art::productSize<T>()(obj); }

    /**REFLEX must call the following constructor
       the constructor takes ownership of T* */
    Wrapper(T *);

    // MUST UPDATE WHEN CLASS IS CHANGED!
    static short Class_Version() { return 10; }

  private:
    virtual
    std::auto_ptr<EDProduct>
    do_makePartner(std::type_info const & wanted_type) const;

    virtual bool isPresent_() const {return present;}

    virtual void do_setPtr(std::type_info const & toType,
                           unsigned long index,
                           void const * &ptr) const;

    virtual void do_getElementAddresses(std::type_info const & toType,
                                        std::vector<unsigned long> const & indices,
                                        std::vector<void const *> &ptr) const;

    bool present;
    //   T const obj;
    T obj;

  };  // Wrapper<>

  template <typename T>
  struct DoMakePartner {
    std::auto_ptr<EDProduct>
    operator()(T const & obj,
               std::type_info const & wanted_wrapper_type) {
      if (typeid(Wrapper<typename T::partner_t>) == wanted_wrapper_type) {
        return obj.makePartner();
      } else {
        throw Exception(errors::LogicError, "makePartner")
          << "Attempted to make partner with inconsistent type information:\n"
          << "Please report to the ART framework developers.\n";
      }
    }
  };

  template <typename T>
  struct DoNotMakePartner {
    std::auto_ptr<EDProduct>
    operator()(T const &,
               std::type_info const &) {
      throw Exception(errors::LogicError, "makePartner")
          << "Attempted to make partner of a product that does not know how!\n"
          << "Please report to the ART framework developers.\n";
    }
  };

  template <typename T>
  std::auto_ptr<EDProduct>
  Wrapper<T>::do_makePartner(std::type_info const & wanted_wrapper) const
  {
    typename std::conditional <detail::has_makePartner_member<T>::value,
      DoMakePartner<T>,
      DoNotMakePartner<T> >::type maybe_maker;
    return maybe_maker(obj, wanted_wrapper);
  }

  template <class T>
  struct DoSetPtr {
    void operator()(T const & obj,
                    const std::type_info & toType,
                    unsigned long index,
                    void const*& ptr) const;
    void operator()(T const & obj,
                    const std::type_info & toType,
                    const std::vector<unsigned long>& index,
                    std::vector<void const *>& ptrs) const;
  };

  template <class T>
  struct DoNotSetPtr {
    void operator()(T const &,
                    const std::type_info &,
                    unsigned long,
                    void const* &) const {
      throw Exception(errors::ProductDoesNotSupportPtr)
          << "The product type "
          << typeid(T).name()
          << "\ndoes not support art::Ptr\n";
    }

    void operator()(T const &,
                    const std::type_info &,
                    const std::vector<unsigned long>&,
                    std::vector<void const *>&) const {
      throw Exception(errors::ProductDoesNotSupportPtr)
          << "The product type "
          << typeid(T).name()
          << "\ndoes not support art::PtrVector\n";
    }

  };

  template <typename T>
  inline
  void Wrapper<T>::do_setPtr(std::type_info const & toType,
                             unsigned long index,
                             void const*& ptr) const
  {
    typename std::conditional < has_setPtr<T>::value,
             DoSetPtr<T>,
             DoNotSetPtr<T> >::type maybe_filler;
    maybe_filler(this->obj, toType, index, ptr);
  }

  template <typename T>
  inline
  void Wrapper<T>::do_getElementAddresses(std::type_info const & toType,
                                          std::vector<unsigned long> const & indices,
                                          std::vector<void const *>& ptrs) const
  {
    typename std::conditional < has_setPtr<T>::value,
             DoSetPtr<T>,
             DoNotSetPtr<T> >::type maybe_filler;
    maybe_filler(this->obj, toType, indices, ptrs);
  }

  // This is an attempt to optimize for speed, by avoiding the copying
  // of large objects of type T. In this initial version, we assume
  // that for any class having a 'swap' member function should call
  // 'swap' rather than copying the object.

  template <typename T>
  struct DoSwap {
    void operator()(T & a, T & b) { a.swap(b); }
  };

  template <typename T>
  struct DoAssign {
    void operator()(T & a, T & b) { a = b; }
  };

  //------------------------------------------------------------
  // Metafunction support for compile-time selection of code used in
  // Wrapper constructor
  //

  namespace detail {
    template <typename T, void (T:: *)(T &)>  struct swap_function;
    template <typename T> no_tag  has_swap_helper(...);
    template <typename T> yes_tag has_swap_helper(swap_function<T, &T::swap> * dummy);

    template<typename T>
    struct has_swap_function {
      static bool const value =
        sizeof(has_swap_helper<T>(0)) == sizeof(yes_tag);
    };

    typedef  std::vector<void const *>  vv_t;
    template <typename T, void (T:: *)(vv_t &)>  struct fillView_function;
    template <typename T> no_tag  has_fillView_helper(...);
    template <typename T> yes_tag has_fillView_helper(fillView_function<T, &T::fillView> * dummy);

    template< typename T >
    struct has_fillView_member {
      static bool const value =
        sizeof(has_fillView_helper<T>(0)) == sizeof(yes_tag);
    };

    template <typename T, size_t (T:: *)() const>  struct size_function;
    template <typename T> no_tag  has_size_helper(...);
    template <typename T> yes_tag has_size_helper(size_function<T, &T::size> * dummy);

    template< typename T >
    struct has_size_member {
      static bool const value =
        sizeof(has_size_helper<T>(0)) == sizeof(yes_tag);
    };

    // CLHEP::HepMatrix and CLHEP::HepSymMatrix have private size data members
    // and therefore require specializations to avoid problems.
    template<>
    struct has_size_member<CLHEP::HepMatrix> {
      static bool const value = false;
    };

    template<>
    struct has_size_member<CLHEP::HepSymMatrix> {
      static bool const value = false;
    };

    template <typename T, std::auto_ptr<EDProduct> (T:: *)() const> struct makePartner_function;
    template <typename T> no_tag  has_makePartner_helper(...);
    template <typename T> yes_tag has_makePartner_helper(makePartner_function<T, &T::makePartner> * dummy);

    template <typename T>
    struct has_makePartner_member {
      static bool const value =
        sizeof(has_makePartner_helper<T>(0)) == sizeof(yes_tag);
    };

  }

  template <class T>
  Wrapper<T>::Wrapper(std::unique_ptr<T> && ptr) :
    EDProduct(),
    present(ptr.get() != 0),
    obj()
  {
    if (present) {
      // The following will call swap if T has such a function,
      // and use assignment if T has no such function.
      typename std::conditional < detail::has_swap_function<T>::value,
               DoSwap<T>,
               DoAssign<T> >::type swap_or_assign;
      swap_or_assign(obj, *ptr);
    }
  }

  template <class T>
  Wrapper<T>::Wrapper(T * ptr) :
    EDProduct(),
    present(ptr != 0),
    obj()
  {
    std::auto_ptr<T> temp(ptr);
    if (present) {
      // The following will call swap if T has such a function,
      // and use assignment if T has no such function.
      typename std::conditional < detail::has_swap_function<T>::value,
               DoSwap<T>,
               DoAssign<T> >::type swap_or_assign;
      swap_or_assign(obj, *ptr);
    }
  }

  template< class T >
  struct fillView<T, true> {
    void operator()( T const          &          product
                    , std::vector<void const *> & view
                   ) {
      product.fillView(view);
    }
  };  // fillView<T>

  template< class T >
  struct fillView<T, false> {
    void operator()( T const                   & /*product*/
                   , std::vector<void const *> & /*view*/
                   ) {
      throw Exception(errors::ProductDoesNotSupportViews)
          << "Product type " << typeid(T).name()
          << " has no fillView() capability\n";
    }
  };  // fillView<T>

  template< >
  struct fillView< std::vector<bool>, false > {
    void operator()( std::vector<bool> const  &  /*product*/
                   , std::vector<void const *> & /*view*/
                   ) {
      throw Exception(errors::ProductDoesNotSupportViews)
          << "Product type std::vector<bool> has no fillView() capability\n";
    }
  };  // fillView<vector<bool>>

  template< class E >
  struct fillView< std::vector<E>, false > {
    void operator()( std::vector<E> const    &   product
                   , std::vector<void const *> & view
                   ) {
      for (typename std::vector<E>::const_iterator b = product.begin()
           , e = product.end()
                 ; b != e; ++b)
      { view.push_back(&*b); }
    }
  };  // fillView<vector<E>>

  template< class E >
  struct fillView< std::list<E>, false > {
    void operator()(std::list<E> const     &    product
                    , std::vector<void const *> & view
                   ) {
      for (typename std::list<E>::const_iterator b = product.begin()
           , e = product.end()
                 ; b != e; ++b)
      { view.push_back(&*b); }
    }
  };  // fillView<list<E>>

  template< class E >
  struct fillView< std::deque<E>, false > {
    void operator()(std::deque<E> const    &    product
                    , std::vector<void const *> & view
                   ) {
      for (typename std::deque<E>::const_iterator b = product.begin()
           , e = product.end()
                 ; b != e; ++b)
      { view.push_back(&*b); }
    }
  };  // fillView<deque<E>>

  template< class E >
  struct fillView< std::set<E>, false > {
    void operator()(std::set<E> const     &     product
                    , std::vector<void const *> & view
                   ) {
      for (typename std::set<E>::const_iterator b = product.begin()
           , e = product.end()
                 ; b != e; ++b)
      { view.push_back(&*b); }
    }
  };  // fillView<set<E>>

  template< class T >
  struct productSize<T, true> {
    std::string
    operator()(T const & obj) const
    { return boost::lexical_cast<std::string>(obj.size()); }
  };

  template< class T >
  struct productSize<T, false> {
    std::string
    operator()(T const &) const
    { return "-"; }
  };

  template< class E >
  struct productSize<std::vector<E>, false>
      : public productSize<std::vector<E>, true>
  { };

  template< class E >
  struct productSize<std::list<E>, false>
      : public productSize<std::list<E>, true>
  { };

  template< class E >
  struct productSize<std::deque<E>, false>
      : public productSize<std::deque<E>, true>
  { };

  template< class E >
  struct productSize<std::set<E>, false>
      : public productSize<std::set<E>, true>
  { };

  template< class E >
  struct productSize<art::PtrVector<E>, false>
      : public productSize<art::PtrVector<E>, true>
  { };

}  // art

namespace art {
  namespace detail {

    template <class element_type>
    void const *
    maybeCastObj(element_type const * address,
                 const std::type_info & iToType)
    {
      if (iToType == typeid(element_type)) {
        return address;
      }
      else {
        using Reflex::Type;
        using Reflex::Object;
        static const Type s_type(Type::ByTypeInfo(typeid(element_type)));
        // The const_cast below is needed because
        // Object's constructor requires a pointer to
        // non-const void, although the implementation does not, of
        // course, modify the object to which the pointer points.
        Object obj(s_type, const_cast<void *>(static_cast<const void *>(address)));
        Object cast = obj.CastObject(Type::ByTypeInfo(iToType));
        if (0 != cast.Address()) {
          return cast.Address(); // returns void*, after pointer adjustment
        }
        else {
          throw cet::exception("TypeConversionError")
              << "art::Wrapper<> : unable to convert type "
              << cet::demangle_symbol(typeid(element_type).name())
              << " to "
              << cet::demangle_symbol(iToType.name())
              << "\n";
        }
      }
    }

  }

  template <class COLLECTION>
  void
  setPtr(COLLECTION const & coll,
         const std::type_info & iToType,
         unsigned long iIndex,
         void const*& oPtr)
  {
    typedef COLLECTION product_type;
    typedef typename product_type::const_iterator iter;
    iter it = coll.begin();
    advance(it, iIndex);
    oPtr = detail::maybeCastObj(detail::GetProduct<product_type>::address(it), iToType);
  }

  namespace detail {

    class value_type_helper {
    public:
      static std::string const & pair_stem() {
        static std::string const pair_stem_s("std::pair<");
        return pair_stem_s;
      }

      static size_t pair_stem_offset() {
        static size_t const pair_stem_offset_s = pair_stem().size();
        return pair_stem_offset_s;
      }

      bool starts_with_pair(std::string const & type_name, size_t pos = 0) {
        return (type_name.compare(pos, pair_stem_offset(), pair_stem()) == 0);
      }

      template <typename T>
      size_t look_past_pair() {
        static std::string const mapped_type = cet::demangle_symbol(typeid(T).name());
        size_t pos = 0;
        while (starts_with_pair(mapped_type, pos)) { pos += pair_stem_offset(); }
        return pos;
      }
    };
  }

  template <class T>
  void
  setPtr(cet::map_vector<T> const & obj,
         const std::type_info & iToType,
         unsigned long iIndex,
         void const*& oPtr)
  {
    typedef cet::map_vector<T> product_type;
    typedef typename product_type::const_iterator iter;
    detail::value_type_helper vh;
    std::string const wanted_type = cet::demangle_symbol(iToType.name());
    static size_t pos = vh.look_past_pair<T>();
    if ((pos < wanted_type.size()) && vh.starts_with_pair(wanted_type, pos)) {
      // Want value_type, not mapped_type;
      iter it = obj.find(cet::map_vector_key(iIndex));
      oPtr = detail::maybeCastObj((it == obj.end()) ? 0 : & (*it), iToType);
    }
    else {
      oPtr = detail::maybeCastObj(obj.getOrNull(cet::map_vector_key(iIndex)), iToType);
    }
  }
}
////////////////////////////////////////////////////////////////////////

namespace art {
  template <class COLLECTION>
  void
  getElementAddresses(COLLECTION const & coll,
                      const std::type_info & iToType,
                      const std::vector<unsigned long>& iIndices,
                      std::vector<void const *>& oPtr)
  {
    typedef COLLECTION product_type;
    typedef typename product_type::const_iterator iter;
    oPtr.reserve(iIndices.size());
    for (std::vector<unsigned long>::const_iterator
         itIndex = iIndices.begin(),
         itEnd = iIndices.end();
         itIndex != itEnd;
         ++itIndex) {
      iter it = coll.begin();
      advance(it, *itIndex);
      oPtr.push_back(detail::maybeCastObj(detail::GetProduct<product_type>::address(it), iToType));
    }
  }

  template <class T>
  void
  getElementAddresses(cet::map_vector<T> const & obj,
                      const std::type_info & iToType,
                      const std::vector<unsigned long>& iIndices,
                      std::vector<void const *>& oPtr)
  {
    typedef cet::map_vector<T> product_type;
    typedef typename product_type::const_iterator iter;
    detail::value_type_helper vh;
    std::string const wanted_type = cet::demangle_symbol(iToType.name());
    static size_t pos = vh.look_past_pair<T>();
    oPtr.reserve(iIndices.size());
    if ((pos < wanted_type.size()) &&
        vh.starts_with_pair(wanted_type, pos)) {
      // Want value_type.
      for (std::vector<unsigned long>::const_iterator
           itIndex = iIndices.begin(),
           itEnd = iIndices.end();
           itIndex != itEnd;
           ++itIndex) {
        iter it = obj.find(cet::map_vector_key(*itIndex));
        oPtr.push_back(detail::maybeCastObj((it == obj.end()) ? 0 : & (*it),
                                            iToType));
      }
    }
    else {
      // Want mapped_type.
      for (std::vector<unsigned long>::const_iterator
           itIndex = iIndices.begin(),
           itEnd = iIndices.end();
           itIndex != itEnd;
           ++itIndex) {
        oPtr.push_back(detail::maybeCastObj
                       (obj.getOrNull(cet::map_vector_key(*itIndex)),
                        iToType));
      }
    }
  }

  template <class T>
  void DoSetPtr<T>::operator()(T const & obj,
                               const std::type_info & toType,
                               unsigned long index,
                               void const* &ptr) const
  {
    // setPtr is the name of an overload set; each concrete collection
    // T should supply a setPtr function, in the same namespace at
    // that in which T is defined, or in the 'art' namespace.
    setPtr(obj, toType, index, ptr);
  }

  template <class T>
  void DoSetPtr<T>::operator()(T const & obj,
                               const std::type_info & toType,
                               const std::vector<unsigned long> &indices,
                               std::vector<void const *> &ptr) const
  {
    // getElementAddresses is the name of an overload set; each
    // concrete collection T should supply a getElementAddresses
    // function, in the same namespace at that in which T is
    // defined, or in the 'art' namespace.
    getElementAddresses(obj, toType, indices, ptr);
  }

}

////////////////////////////////////////////////////////////////////////

#endif /* art_Persistency_Common_Wrapper_h */

// Local Variables:
// mode: c++
// End:
