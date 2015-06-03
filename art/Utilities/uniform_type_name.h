#ifndef CETLIB_UNIFORM_TYPE_NAME_H
#define CETLIB_UNIFORM_TYPE_NAME_H

#include "cetlib/demangle.h"

#include <string>
#include <typeinfo>

namespace art {
  /// \fn uniform_type_name
  ///
  /// \brief Calculate the uniform type name of the provided type.
  ///
  /// The CXXABI demangle function can return an answer which is
  /// compiler-dependent. This function attempts to manipulate that
  /// compiler-dependent name into a form which is compatible with that
  /// produced by Reflex::Type::Name(Reflex::SCOPED), for historical
  /// reasons.
  ///
  /// This has been verified to produce good output with GCC 4.9.2.
  ///
  /// \returns the uniform type name.
  ///
  /// \param[in] tid A reference to the std::type_info for the type in
  /// question.
  std::string uniform_type_name(std::type_info const & tid);
  ///
  /// \param[in] name The already-demangled name for a type.
  std::string uniform_type_name(std::string name);
}

inline
std::string
art::uniform_type_name(std::type_info const & tid)
{
  return uniform_type_name(cet::demangle_symbol(tid.name()));
}

#endif /* CETLIB_UNIFORM_TYPE_NAME_H */

// Local variables:
// mode:c++
// End:
