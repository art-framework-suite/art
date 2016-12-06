#ifndef art_Utilities_detail_tool_type_h
#define art_Utilities_detail_tool_type_h

#include "canvas/Utilities/Exception.h"
#include "cetlib/BasicPluginFactory.h"

#include <functional>
#include <memory>
#include <string>
#include <type_traits>

namespace fhicl { class ParameterSet; }

namespace art {
  namespace detail {

    template <typename T, typename = void>
    struct tool_type;

    template <typename T>
    struct tool_type<T, std::enable_if_t<std::is_class<T>::value>>
    {
      using return_type = std::unique_ptr<T>;

      static auto make_plugin(cet::BasicPluginFactory& factory,
                              std::string const& libspec,
                              fhicl::ParameterSet const& pset)
      {
        return factory.makePlugin<std::unique_ptr<T>>(libspec, pset);
      }
    };

    template <typename T>
    struct tool_type<T, std::enable_if_t<std::is_function<T>::value>>
    {
      using return_type = std::function<T>;

      static auto make_plugin(cet::BasicPluginFactory& factory,
                              std::string const& libspec,
                              fhicl::ParameterSet const&,
                              std::string const& function_plugin_type)
      {
        auto const pluginType = factory.pluginType(libspec);
        return pluginType == function_plugin_type ?
          factory.makePlugin<T>(libspec) :
          throw Exception(errors::Configuration, "tool_type::make_plugin: ")
            << "Unrecognized function-tool type \"" << function_plugin_type << "\" for plugin \"" << libspec << "\".\n"
            << "Allowed function-tool type for above plugin is: \"" << pluginType << "\".\n";

      }
    };

  }
}

#endif

// Local variables:
// mode: c++
// End:
