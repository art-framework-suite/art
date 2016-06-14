#ifndef art_Persistency_Provenance_ReflexTools_h
#define art_Persistency_Provenance_ReflexTools_h

/*----------------------------------------------------------------------

ReflexTools provides a small number of Reflex-based tools, used in
the CMS event model.

----------------------------------------------------------------------*/

#include "art/Utilities/WrappedClassName.h"

#include "Reflex/Object.h"
#include "Reflex/Type.h"

#include <ostream>
#include <string>
#include <vector>

namespace art
{

  bool
  find_nested_type_named(std::string const& nested_type,
                         Reflex::Type const& type_to_search,
                         Reflex::Type& found_type);

  bool
  value_type_of(Reflex::Type const& t, Reflex::Type& found_type);

  bool
  mapped_type_of(Reflex::Type const& t, Reflex::Type& found_type);

  void checkDictionaries(std::string const& name, bool noComponents = false);
  void reportFailedDictionaryChecks();

  void public_base_classes(const Reflex::Type& type,
                           std::vector<Reflex::Type>& baseTypes);

  /// Try to convert the un-typed pointer raw (which we promise is a
  /// pointer to an object whose dynamic type is denoted by
  /// dynamicType) to a pointer of type T. This is like the
  /// dynamic_cast operator, in that it can do pointer adjustment (in
  /// cases of multiple inheritance), and will return 0 if T is
  /// neither the same type as nor a public base of the C++ type
  /// denoted by dynamicType.

  // It would be nice to use void const* for the type of 'raw', but
  // the Reflex interface for creating an Object will not allow that.

  template <class T>
  T const*
  reflex_cast(void* raw, Reflex::Type const& dynamicType)
  {
    static const Reflex::Type
      toType(Reflex::Type::ByTypeInfo(typeid(T)));

    Reflex::Object obj(dynamicType, raw);
    return static_cast<T const*>(obj.CastObject(toType).Address());

    // This alternative implementation of reflex_cast would allow us
    // to remove the compile-time depenency on Reflex/Type.h and
    // Reflex/Object.h, at the cost of some speed.
    //
    //     return static_cast<T const*>(reflex_pointer_adjust(raw,
    //                                                 dynamicType,
    //                                                 typeid(T)));
  }

  // The following function should not now be used. It is here in case
  // we need to get rid of the compile-time dependency on
  // Reflex/Type.h and Reflex/Object.h introduced by the current
  // implementation of reflex_cast (above). If we have to be rid of
  // that dependency, the alternative implementation of reflex_cast
  // uses this function, at the cost of some speed: repeated lookups
  // of the same Reflex::Type object for the same type will have
  // to be made.

  /// Take an un-typed pointer raw (which we promise is a pointer to
  /// an object whose dynamic type is denoted by dynamicType), and
  /// return a raw pointer that is appropriate for referring to an
  /// object whose type is denoted by toType. This performs any
  /// pointer adjustment needed for dealing with base class
  /// sub-objects, and returns 0 if the type denoted by toType is
  /// neither the same as, nor a public base of, the type denoted by
  /// dynamicType.

  void const*
  reflex_pointer_adjust(void* raw,
                        Reflex::Type const& dynamicType,
                        std::type_info const& toType);

  std::string cint_wrapper_name(std::string const &className);

  Reflex::Type type_of_template_arg(Reflex::Type const &template_instance,
                                    size_t arg_index);

  bool is_instantiation_of(Reflex::Type const &in,
                           std::string const &template_name);
}

#endif /* art_Persistency_Provenance_ReflexTools_h */

// Local Variables:
// mode: c++
// End:
