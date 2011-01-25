#ifndef Art_Framework_Core_open_config_h
#define Art_Framework_Core_open_config_h

#include <string>

namespace art {
   bool find_config(std::string const &filename,
                    std::string const &search_path_spec,
                    std::string &full_path);
}

#endif
