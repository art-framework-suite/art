#ifndef art_Framework_Core_detail_ImplicitConfigs_h
#define art_Framework_Core_detail_ImplicitConfigs_h

#include <set>
#include <string>

namespace art::detail {

  struct ModuleConfig {
    static fhicl::Name
    plugin_type()
    {
      return fhicl::Name{"module_type"};
    }
    struct IgnoreKeys {
      std::set<std::string>
      operator()()
      {
        return {"module_label"};
      }
    };
  };

  struct PluginConfig {
    static fhicl::Name
    plugin_type()
    {
      return fhicl::Name{"plugin_type"};
    }
    struct IgnoreKeys {
      std::set<std::string>
      operator()()
      {
        return {"plugin_label"};
      }
    };
  };

} // namespace art::detail

#endif /* art_Framework_Core_detail_ImplicitConfigs_h */

// Local variables:
// mode: c++
// End:
