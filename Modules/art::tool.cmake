#[================================================================[.rst:
X
-
#]================================================================]
include_guard()

cmake_minimum_required(VERSION 3.18.2...3.27 FATAL_ERROR)

include(BasicPlugin)

macro(art::tool NAME)
  cmake_parse_arguments(_at "" "SUFFIX" "" ${ARGN})
  if (TARGET art_plugin_types::tool)
    set(_art_tool_deps LIBRARIES REG art_plugin_types::tool)
    if (TARGET art_plugin_types::tool_interface)
      list(APPEND _art_tool_deps
        CONDITIONAL art_plugin_types::tool_interface)
    else()
      list(APPEND _art_tool_deps
        CONDITIONAL fhiclcpp::types fhiclcpp::fhiclcpp)
    endif()
  else()
    # Older art suites.
    set(_art_tool_deps LIBRARIES CONDITIONAL
      art_Utilities
      fhiclcpp
      cetlib
      cetlib_except
      Boost::filesystem
    )
  endif()
  if ("${_at_SUFFIX}" STREQUAL "")
    set(_at_SUFFIX tool)
  endif()
  basic_plugin(${NAME} ${_at_SUFFIX} ${_at_UNPARSED_ARGUMENTS} ${_art_tool_deps})
  unset(_art_tool_deps)
endmacro()
