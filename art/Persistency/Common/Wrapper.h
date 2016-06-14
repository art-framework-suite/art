#ifndef art_Persistency_Common_Wrapper_h
#define art_Persistency_Common_Wrapper_h

////////////////////////////////////////////////////////////////////////
// Wrapper: A template wrapper around EDProducts to hold the product ID.
//
////////////////////////////////////////////////////////////////////////

#include "art/Persistency/Common/EDProduct.h"
#include "cetlib/demangle.h"

#include <memory>

#ifndef __GCCXML__
// Required for specializations of has_size_member<T>, below.
namespace CLHEP {
  class HepMatrix;
  class HepSymMatrix;
}

#include <string>
#include <vector>
#endif

namespace art {
  template <typename T> class Wrapper;

#ifndef __GCCXML__
  // Implementation detail declarations.
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

  template< typename T, bool = detail::has_fillView_member<T>::value >
  struct fillView;

  template< typename T, bool = detail::has_size_member<T>::value >
  struct productSize;

  template <typename T>
  struct DoMakePartner;

  template <typename T>
  struct DoNotMakePartner;

  template <typename T>
  struct DoSetPtr;

  template <typename T>
  struct DoNotSetPtr;

#endif
}

////////////////////////////////////////////////////////////////////////
// Definition of art::Wrapper<T>
template <typename T>
class art::Wrapper : public art::EDProduct {
public:
  Wrapper();

#ifndef __GCCXML__
  explicit Wrapper(std::unique_ptr<T> ptr);
#endif

  virtual ~Wrapper();

#ifndef __GCCXML__
  T const * product() const;
  T const * operator->() const;

  virtual void fillView(std::vector<void const *> & view) const;

  virtual std::string productSize() const;
#endif

  // MUST UPDATE WHEN CLASS IS CHANGED!
  static short Class_Version() { return 10; }

private:
  virtual
  std::unique_ptr<EDProduct>
  do_makePartner(std::type_info const & wanted_type) const;

  virtual bool isPresent_() const {return present;}

  virtual void do_setPtr(std::type_info const & toType,
                         unsigned long index,
                         void const * &ptr) const;

  virtual void do_getElementAddresses(std::type_info const & toType,
                                      std::vector<unsigned long> const & indices,
                                      std::vector<void const *> &ptr) const;

#ifndef __GCCXML__
  T && refOrThrow(T * ptr);
#endif

  bool present;
  //   T const obj;
  T obj;

};  // Wrapper<>

////////////////////////////////////////////////////////////////////////
// Implementation details.

#ifndef __GCCXML__

#include "art/Persistency/Common/GetProduct.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Common/getElementAddresses.h"
#include "art/Persistency/Common/setPtr.h"

#include "art/Persistency/Common/traits.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/detail/metaprogramming.h"
#include "boost/lexical_cast.hpp"
#include "cpp0x/memory"
#include "cpp0x/type_traits"

#include <deque>
#include <list>
#include <set>
#include <string>
#include <typeinfo>
#include <vector>

////////////////////////////////////////////////////////////////////////
// Wrapper member functions.
template <typename T>
art::Wrapper<T>::
Wrapper()
  :
  EDProduct(),
  present(false),
  obj()
{
}

template <typename T>
art::Wrapper<T>::
Wrapper(std::unique_ptr<T> ptr) :
  EDProduct(),
  present(ptr.get() != 0),
  obj(refOrThrow(ptr.get()))
{
}

template <typename T>
art::Wrapper<T>::
~Wrapper()
{
}

template <typename T>
T const *
art::Wrapper<T>::
product() const
{
  return (present ? &obj : nullptr);
}

template <typename T>
T const *
art::Wrapper<T>::
operator->() const
{
  return product();
}

template <typename T>
void
art::Wrapper<T>::
fillView(std::vector<void const *> & view) const
{
  art::fillView<T>()(obj, view);
}

template <typename T>
std::string
art::Wrapper<T>::
productSize() const
{
  return art::productSize<T>()(obj);
}

template <typename T>
std::unique_ptr<art::EDProduct>
art::Wrapper<T>::
do_makePartner(std::type_info const & wanted_wrapper) const
{
  typename std::conditional <detail::has_makePartner_member<T>::value, DoMakePartner<T>, DoNotMakePartner<T> >::type maybe_maker;
  return maybe_maker(obj, wanted_wrapper);
}

template <typename T>
inline
void
art::Wrapper<T>::
do_setPtr(std::type_info const & toType,
          unsigned long index,
          void const*& ptr) const
{
  typename std::conditional < has_setPtr<T>::value, DoSetPtr<T>, DoNotSetPtr<T> >::type maybe_filler;
  maybe_filler(this->obj, toType, index, ptr);
}

template <typename T>
inline
void
art::Wrapper<T>::
do_getElementAddresses(std::type_info const & toType,
                       std::vector<unsigned long> const & indices,
                       std::vector<void const *>& ptrs) const
{
  typename std::conditional < has_setPtr<T>::value, DoSetPtr<T>, DoNotSetPtr<T> >::type maybe_filler;
  maybe_filler(this->obj, toType, indices, ptrs);
}

template <typename T>
inline
T &&
art::Wrapper<T>::
refOrThrow(T * ptr)
{
  if (ptr) {
    return std::move(*ptr);
  } else {
    throw Exception(errors::NullPointerError)
      << "Attempt to construct "
      << cet::demangle_symbol(typeid(*this).name())
      << " from nullptr.\n";
  }
}

////////////////////////////////////////////////////////////////////////
// Metafunction support for compile-time selection of code used in
// Wrapper implementation.

namespace art {
  namespace detail {
    typedef  std::vector<void const *>  vv_t;
    template <typename T, void (T:: *)(vv_t &)>  struct fillView_function;
    template <typename T> no_tag  has_fillView_helper(...);
    template <typename T> yes_tag has_fillView_helper(fillView_function<T, &T::fillView> * dummy);

    template <typename T, size_t (T:: *)() const>  struct size_function;
    template <typename T> no_tag  has_size_helper(...);
    template <typename T> yes_tag has_size_helper(size_function<T, &T::size> * dummy);

    template <typename T, std::unique_ptr<EDProduct> (T:: *)() const> struct makePartner_function;
    template <typename T> no_tag  has_makePartner_helper(...);
    template <typename T> yes_tag has_makePartner_helper(makePartner_function<T, &T::makePartner> * dummy);
  }
}

template< typename T >
struct art::detail::has_fillView_member {
  static bool const value =
    sizeof(has_fillView_helper<T>(0)) == sizeof(yes_tag);
};

template< typename T >
struct art::detail::has_size_member {
  static bool const value =
    sizeof(has_size_helper<T>(0)) == sizeof(yes_tag);
};

// CLHEP::HepMatrix and CLHEP::HepSymMatrix have private size data members
// and therefore require specializations to avoid problems.
template<>
struct art::detail::has_size_member<CLHEP::HepMatrix> {
  static bool const value = false;
};

template<>
struct art::detail::has_size_member<CLHEP::HepSymMatrix> {
  static bool const value = false;
};


template <typename T>
struct art::detail::has_makePartner_member {
  static bool const value =
    sizeof(has_makePartner_helper<T>(0)) == sizeof(yes_tag);
};

namespace art {

  template <typename T >
  struct fillView<T, true> {
    void operator()(T const & product,
                    std::vector<void const *> & view) {
      product.fillView(view);
    }
  };  // fillView<T>

  template <typename T >
  struct fillView<T, false> {
    void operator()(T const &,
                    std::vector<void const *> &) {
      throw Exception(errors::ProductDoesNotSupportViews)
        << "Product type " << cet::demangle_symbol(typeid(T).name())
        << " has no fillView() capability.\n";
    }
  };  // fillView<T>

  template <>
  struct fillView<std::vector<bool>, false > {
    void operator()(std::vector<bool> const &,
                    std::vector<void const *> &) {
      throw Exception(errors::ProductDoesNotSupportViews)
          << "Product type std::vector<bool> has no fillView() capability.\n";
    }
  };  // fillView<vector<bool>>

  template <class E >
  struct fillView< std::vector<E>, false > {
    void operator()(std::vector<E> const & product,
                    std::vector<void const *> & view) {
    for (auto const & p : product) {
        view.push_back(&p);
      }
    }
  };  // fillView<vector<E>>

  template <class E >
  struct fillView< std::list<E>, false > {
    void operator()(std::list<E> const & product,
                    std::vector<void const *> & view) {
    for (auto const & p : product) {
        view.push_back(&p);
      }
    }
  };  // fillView<list<E>>

  template <class E >
  struct fillView< std::deque<E>, false > {
    void operator()(std::deque<E> const & product,
                    std::vector<void const *> & view) {
      for (auto const & p : product) {
        view.push_back(&p);
      }
    }
  };  // fillView<deque<E>>

  template <class E >
  struct fillView< std::set<E>, false > {
    void operator()(std::set<E> const & product,
                    std::vector<void const *> & view) {
      for (auto const & p : product) {
        view.push_back(&p);
      }
    }
  };  // fillView<set<E>>

  template <class E>
  struct fillView<cet::map_vector<E>, false> {
    void operator() (cet::map_vector<E> const & product,
                     std::vector<void const *> & view)
      {
        for (auto const & p : product) {
          view.push_back(&p.second);
        }
      }
  }; // fillView<cet::map_vector<E>>

  template <typename T >
  struct productSize<T, true> {
    std::string
    operator()(T const & obj) const {
      return boost::lexical_cast<std::string>(obj.size());
    }
  };

  template <typename T >
  struct productSize<T, false> {
    std::string
    operator()(T const &) const
    { return "-"; }
  };

  template <class E >
  struct productSize<std::vector<E>, false>
      : public productSize<std::vector<E>, true>
  { };

  template <class E >
  struct productSize<std::list<E>, false>
      : public productSize<std::list<E>, true>
  { };

  template <class E >
  struct productSize<std::deque<E>, false>
      : public productSize<std::deque<E>, true>
  { };

  template <class E >
  struct productSize<std::set<E>, false>
      : public productSize<std::set<E>, true>
  { };

  template <class E >
  struct productSize<PtrVector<E>, false>
      : public productSize<PtrVector<E>, true>
  { };

  template <class E >
  struct productSize<cet::map_vector<E>, false>
    : public productSize<cet::map_vector<E>, true>
  { };

  template <typename T>
  struct DoMakePartner {
    std::unique_ptr<EDProduct>
    operator()(T const & obj,
               std::type_info const & wanted_wrapper_type) const {
      if (typeid(Wrapper<typename T::partner_t>) == wanted_wrapper_type) {
        return obj.makePartner();
      }
      else {
        throw Exception(errors::LogicError, "makePartner")
            << "Attempted to make partner with inconsistent type information:\n"
            << "Please report to the ART framework developers.\n";
      }
    }
  };

  template <typename T>
  struct DoNotMakePartner {
    std::unique_ptr<EDProduct>
    operator()(T const &,
               std::type_info const &) const {
      throw Exception(errors::LogicError, "makePartner")
          << "Attempted to make partner of a product that does not know how!\n"
          << "Please report to the ART framework developers.\n";
    }
  };

  template <typename T> struct DoSetPtr {
    void operator()(T const & obj,
                    const std::type_info & toType,
                    unsigned long index,
                    void const*& ptr) const;
    void operator()(T const & obj,
                    const std::type_info & toType,
                    const std::vector<unsigned long>& index,
                    std::vector<void const *>& ptrs) const;
  };

  template <typename T>
  struct DoNotSetPtr {
    void operator()(T const &,
                    const std::type_info &,
                    unsigned long,
                    void const* &) const {
      throw Exception(errors::ProductDoesNotSupportPtr)
          << "The product type "
          << cet::demangle_symbol(typeid(T).name())
          << "\ndoes not support art::Ptr\n";
    }

    void operator()(T const &,
                    const std::type_info &,
                    const std::vector<unsigned long>&,
                    std::vector<void const *>&) const {
      throw Exception(errors::ProductDoesNotSupportPtr)
          << "The product type "
          << cet::demangle_symbol(typeid(T).name())
          << "\ndoes not support art::PtrVector\n";
    }
  };

  template <typename T>
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

  template <typename T>
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
#endif /* __GCCXML__ */

#endif /* art_Persistency_Common_Wrapper_h */

// Local Variables:
// mode: c++
// End:
