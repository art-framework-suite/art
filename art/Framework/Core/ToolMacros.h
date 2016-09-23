#ifndef art_Framework_Core_ToolMacros_h
#define art_Framework_Core_ToolMacros_h

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

#include "art/Utilities/BasicHelperMacros.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <type_traits>

// Produce the injected functions
#define DEFINE_ART_TOOL_CLASS(tool)                                     \
  extern "C" {                                                          \
    PROVIDE_FILE_PATH()                                                 \
    std::enable_if_t<std::is_class< tool >::value, std::unique_ptr< tool > > \
    makeTool(fhicl::ParameterSet const& pset)                           \
    {                                                                   \
      return std::make_unique< tool >(pset);                            \
    }                                                                   \
  }

#define DEFINE_ART_TOOL_FUNCTION(tool)                                  \
  extern "C" {                                                          \
    PROVIDE_FILE_PATH()                                                 \
    auto toolFunction = tool;                                           \
  }

#endif /* art_Framework_Core_ToolMacros_h */

// Local Variables:
// mode: c++
// End:
