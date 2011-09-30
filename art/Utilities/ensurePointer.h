#ifndef art_Utilities_ensurePointer_h
#define art_Utilities_ensurePointer_h

// Ensure we can get a desired pointer type from a compatible iterator.

// In order to allow new iterator value types (e.g. smart pointers) to
// be converted to pointers. specialize art::detail::EnsurePointer. See
// art/Persistency/Common/Ptr.h for an example of this. Also see
// test/Utilities/ensurePointer.h for testing coverage illustration.

#include "art/Utilities/Exception.h"
#include "cetlib/demangle.h"
#include "cpp0x/type_traits"

#include "boost/mpl/assert.hpp"
#include "boost/type_traits.hpp"

#include <iterator>

namespace art {
  template <typename WANTED_POINTER, typename InputIterator>
  WANTED_POINTER
  ensurePointer(InputIterator it);

  namespace detail {
    template <typename T1, typename T2>
    struct are_cv_compatible {
      typedef typename std::remove_cv<typename std::remove_pointer<T1>::type>::type T1P;
      typedef typename std::remove_cv<typename std::remove_pointer<T2>::type>::type T2P;
      static bool const value =  std::is_base_of<T1P, T2P>::value || std::is_same<T1P, T2P>::value;
    };

    template <typename TO, typename FROM>
    typename std::enable_if<are_cv_compatible<TO, FROM>::value, typename std::add_pointer<typename std::remove_pointer<TO>::type>::type>::type
    addr(FROM & from) { return &from; }

    template <typename TO, typename FROM>
    typename std::enable_if < !are_cv_compatible<TO, FROM>::value && are_cv_compatible<FROM, TO>::value, typename std::add_pointer<typename std::remove_pointer<TO>::type>::type >::type
    addr(FROM & from) { return &dynamic_cast<typename boost::add_reference<typename std::remove_pointer<TO>::type>::type>(from); }

    template <typename TO, typename FROM>
    struct EnsurePointer {
      TO
      operator()(FROM & from) const { return detail::addr<TO>(from); }
      TO
      operator()(FROM const & from) const { return detail::addr<TO>(from); }
    };

    template <typename TO, typename PFROM>
    struct EnsurePointer<TO, PFROM *> {
      TO
      operator()(PFROM * from) const { return detail::addr<TO>(*from); }
    };
  }
}

template <typename WANTED_POINTER, typename InputIterator>
inline
WANTED_POINTER
art::ensurePointer(InputIterator it) try
{
  BOOST_MPL_ASSERT((std::is_pointer<WANTED_POINTER>));
  return detail::EnsurePointer<WANTED_POINTER, typename std::iterator_traits<InputIterator>::value_type>()(*it);
}
catch (std::bad_cast &)
{
  throw Exception(errors::LogicError, "ensurePointer")
      << "Iterator value type "
      << cet::demangle_symbol(typeid(typename std::iterator_traits<InputIterator>::value_type).name())
      << " and wanted pointer type "
      << cet::demangle_symbol(typeid(WANTED_POINTER).name())
      << " are incompatible.\n";
}

#endif /* art_Utilities_ensurePointer_h */

// Local Variables:
// mode: c++
// End:
