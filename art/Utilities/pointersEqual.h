#ifndef art_Utilities_pointersEqual_h
#define art_Utilities_pointersEqual_h
// Compare two pointers taking into account possible multiple inheritance

#include "canvas/Utilities/Exception.h"

#include <type_traits>

namespace art {

  template <typename T1, typename T2>
  concept compatible = std::same_as<T1, T2> ||
                    std::derived_from<T1, T2> ||
                    std::derived_from<T2, T1>;

  template <typename T1, typename T2>
    requires (compatible<T1, T2>)
  bool pointersEqual(T1* t1, T2* t2)
  {
    return t1 == t2;
  }

  // Not compatible.
  template <typename T1, typename T2>
    requires (!compatible<T1, T2>)
  bool pointersEqual(T1*, T2*)
  {
    throw art::Exception(art::errors::LogicError)
      << "Tried to compare two incompatible pointers.\n";
  }
} // namespace art

#endif /* art_Utilities_pointersEqual_h */

// Local Variables:
// mode: c++
// End:
