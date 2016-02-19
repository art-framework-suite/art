#ifndef art_Utilities_pointersEqual_h
#define art_Utilities_pointersEqual_h
// Compare two pointers taking into account possible multiple inheritance

#include "canvas/Utilities/Exception.h"

#include <type_traits>

namespace art {
  template <typename T1, typename T2>
  inline
  typename std::enable_if < std::is_same<T1, T2>::value || std::is_base_of<T1, T2>::value || std::is_base_of<T2, T1>::value, bool >::type
  pointersEqual(T1 * t1, T2 * t2) { return t1 == t2; }

  // Not compatible.
  template <typename T1, typename T2>
  typename std::enable_if < !std::is_same<T1, T2>::value && !std::is_base_of<T1, T2>::value && !std::is_base_of<T2, T1>::value, bool >::type
  pointersEqual(T1 * t1, T2 * t2);
}

template <typename T1, typename T2>
typename std::enable_if < !std::is_same<T1, T2>::value && !std::is_base_of<T1, T2>::value && !std::is_base_of<T2, T1>::value, bool >::type
art::pointersEqual(T1 *, T2 *)
{
  throw art::Exception(art::errors::LogicError)
      << "Tried to compare two incompatible pointers.\n";
}

#endif /* art_Utilities_pointersEqual_h */

// Local Variables:
// mode: c++
// End:
