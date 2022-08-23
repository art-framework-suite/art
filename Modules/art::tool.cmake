#[================================================================[.rst:
X
=
#]================================================================]
include_guard()

cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

include(BasicPlugin)

macro(art::tool NAME)
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
  basic_plugin(${NAME} tool ${ARGN} ${_art_tool_deps})
  unset(_art_tool_deps)
endmacro()
