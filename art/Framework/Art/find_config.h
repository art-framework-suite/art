// ======================================================================
//
// find_config
//
// ======================================================================

#ifndef art_Framework_Art_find_config_h
#define art_Framework_Art_find_config_h

#include <string>

namespace art {
  bool find_config(std::string const& filename,
                   std::string const& search_path_spec,
                   std::string& full_path);
}

#endif /* art_Framework_Art_find_config_h */

// Local Variables:
// mode: c++
// End:
