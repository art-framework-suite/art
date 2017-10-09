#ifndef art_Utilities_ToolMacros_h
#define art_Utilities_ToolMacros_h

////////////////////////////////////////////////////////////////////////
// ToolMacros
//
// Defines the macro DEFINE_ART_TOOL(<tool_classname>) to be used in
// XXX_tool.cc to declare art tools.
//
// Note: Libraries that include these symbol definitions cannot be
// linked into a main program as other libraries are.  This is because
// the "one definition" rule would be violated.
//
////////////////////////////////////////////////////////////////////////

#include "cetlib/PluginTypeDeducer.h"
#include "cetlib/ProvideFilePathMacro.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/AllowedConfigurationMacro.h"
#include "fhiclcpp/types/Atom.h"

#include <memory>
#include <string>
#include <type_traits>

namespace art {
  namespace detail {

    struct ToolConfig {
      fhicl::Atom<std::string> tool_type{fhicl::Name("tool_type")};
    };

  } // namespace detail
} // namespace art

#define ART_PROVIDE_ALLOWED_CONFIGURATION_FUNCTION_TOOL()                      \
  std::unique_ptr<fhicl::ConfigurationTable> allowed_configuration(            \
    std::string const& name)                                                   \
  {                                                                            \
    return std::make_unique<fhicl::WrappedTable<art::detail::ToolConfig>>(     \
      fhicl::Name{name});                                                      \
  }

// Produce the injected functions
#define DEFINE_ART_CLASS_TOOL(tool)                                            \
  extern "C" {                                                                 \
  CET_PROVIDE_FILE_PATH()                                                      \
  FHICL_PROVIDE_ALLOWED_CONFIGURATION(tool)                                    \
  std::string                                                                  \
  toolType()                                                                   \
  {                                                                            \
    return "class";                                                            \
  }                                                                            \
  std::enable_if_t<std::is_class<tool>::value, std::unique_ptr<tool>>          \
  makeTool(fhicl::ParameterSet const& pset)                                    \
  {                                                                            \
    return std::make_unique<tool>(pset);                                       \
  }                                                                            \
  }

#define DEFINE_ART_FUNCTION_TOOL(tool, type)                                   \
  extern "C" {                                                                 \
  CET_PROVIDE_FILE_PATH()                                                      \
  ART_PROVIDE_ALLOWED_CONFIGURATION_FUNCTION_TOOL()                            \
  std::string                                                                  \
  toolType()                                                                   \
  {                                                                            \
    return type;                                                               \
  }                                                                            \
  auto toolFunction = tool;                                                    \
  }

#endif /* art_Utilities_ToolMacros_h */

// Local Variables:
// mode: c++
// End:
