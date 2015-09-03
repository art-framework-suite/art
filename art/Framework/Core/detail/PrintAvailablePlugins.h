#ifndef art_Framework_Core_detail_PrintAvailablePlugins_h
#define art_Framework_Core_detail_PrintAvailablePlugins_h

#include <string>
#include <vector>

namespace art {
  namespace detail {

    void print_available_modules ();
    void print_available_services();

    void print_module_description (std::vector<std::string> const& mods);
    void print_service_description(std::vector<std::string> const& svcs);

  }
}

#endif /* art_Framework_Core_detail_PrintAvailablePlugins_h */

// Local variables:
// mode: c++
// End:
