#ifndef art_Utilities_intermediate_table_helpers_h
#define art_Utilities_intermediate_table_helpers_h

#include <string>

namespace fhicl {
   class intermediate_table;
}

namespace art {
   void ensure_table(fhicl::intermediate_table &table,
                     std::string const &fhicl_spec);
}

#endif /* art_Utilities_intermediate_table_helpers_h */

// Local Variables:
// mode: c++
// End:
