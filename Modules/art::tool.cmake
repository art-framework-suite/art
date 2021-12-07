#[================================================================[.rst:
X
=
#]================================================================]
include_guard()

cmake_policy(PUSH)
cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

include(BasicPlugin)

macro(art::tool NAME)
  if (TARGET art_plugin_types::tool)
    set(_art_tool_deps LIBRARIES REG art_plugin_types::tool)
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

cmake_policy(POP)
