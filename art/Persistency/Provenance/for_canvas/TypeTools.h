#ifndef art_Persistency_Provenance_TypeTools_h
#define art_Persistency_Provenance_TypeTools_h
// vim: set sw=2:

//
//  TypeTools provides a small number of Reflex-based tools, used in
//  the CMS event model.
//

#include "canvas/Persistency/Provenance/TypeWithDict.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "TClass.h"
#include <ostream>
#include <string>
#include <vector>

namespace art {

bool
find_nested_type_named(std::string const& nested_type,
                       TClass* const type_to_search,
                       TypeWithDict& found_type);

bool
value_type_of(TClass* t, TypeWithDict& found_type);

bool
mapped_type_of(TClass* t, TypeWithDict& found_type);

void
checkDictionaries(std::string const& name, bool recursive = false,
                  int level = 0);

void
reportFailedDictionaryChecks();

void
public_base_classes(TClass* cl, std::vector<TClass*>& baseTypes);

TClass*
type_of_template_arg(TClass* template_instance, size_t desired_arg);

bool
is_instantiation_of(TClass* cl, std::string const& template_name);

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif // art_Persistency_Provenance_TypeTools_h
