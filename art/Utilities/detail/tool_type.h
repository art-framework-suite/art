#ifndef art_Utilities_detail_tool_type_h
#define art_Utilities_detail_tool_type_h

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

      static std::string plugin_function_name() { return "makeTool"; }
    };

    template <typename T>
    struct tool_type<T, std::enable_if_t<std::is_function<T>::value>>
    {
      using return_type = std::function<T>;

      static auto make_plugin(cet::BasicPluginFactory& factory,
                              std::string const& libspec,
                              fhicl::ParameterSet const&)
      {
        return factory.makePlugin<T>(libspec);
      }

      static std::string plugin_function_name() { return "toolFunction"; }
    };

  }
}

#endif

// Local variables:
// mode: c++
// End:
