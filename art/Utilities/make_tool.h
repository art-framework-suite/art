#ifndef art_Utilities_Tool_h
#define art_Utilities_Tool_h

#include "art/Utilities/PluginSuffixes.h"
#include "art/Utilities/detail/tool_type.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/BasicPluginFactory.h"
#include "cetlib/detail/wrapLibraryManagerException.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace art {

  template <typename T>
  using tool_return_type = typename detail::tool_type<T>::return_type;

  template <typename T>
  std::enable_if_t<std::is_class<T>::value, tool_return_type<T>>
  make_tool(fhicl::ParameterSet const& pset)
  {
    cet::BasicPluginFactory factory {Suffixes::tool(), "makeTool"};
    std::string const libspec {pset.get<std::string>("tool_type")};
    tool_return_type<T> result;
    try {
      result = detail::tool_type<T>::make_plugin(factory, libspec, pset);
    }
    catch (cet::exception const& e) {
      throw Exception(errors::Configuration, "make_tool: ", e)
        << "Exception caught while processing plugin spec: " << libspec << '\n';
    }
    return result;
  }

  template <typename T>
  std::enable_if_t<std::is_function<T>::value, tool_return_type<T>>
  make_tool(fhicl::ParameterSet const& pset, std::string const& function_tool_type)
  {
    cet::BasicPluginFactory factory {Suffixes::tool(), "toolFunction", "toolType"};
    std::string const libspec {pset.get<std::string>("tool_type")};
    tool_return_type<T> result;
    try {
      result = detail::tool_type<T>::make_plugin(factory, libspec, pset, function_tool_type);
    }
    catch (cet::exception const& e) {
      throw Exception(errors::Configuration, "make_tool: ", e)
        << "Exception caught while processing plugin spec: " << libspec << '\n';
    }
    return result;
  }

  template <typename T, typename TableConfig>
  inline
  std::enable_if_t<std::is_class<T>::value, tool_return_type<T>>
  make_tool(TableConfig const& tc)
  {
    return make_tool<T>(tc.get_PSet());
  }

  template <typename T, typename TableConfig>
  inline
  std::enable_if_t<std::is_function<T>::value, tool_return_type<T>>
  make_tool(TableConfig const& tc, std::string const& function_tool_type)
  {
    return make_tool<T>(tc.get_PSet(), function_tool_type);
  }

}

#endif

// Local variables:
// mode: c++
// End:
