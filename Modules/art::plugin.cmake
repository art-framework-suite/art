#[================================================================[.rst:
X
=
#]================================================================]
include_guard()

cmake_policy(PUSH)
cmake_minimum_required(VERSION 3.18.2...3.20 FATAL_ERROR)

include(BasicPlugin)

macro(art::plugin NAME)
  if (TARGET art_plugin_support::plugin_config_macros)
    set(_art_plugin_deps LIBRARIES REG
      art_plugin_support::plugin_config_macros
      art_plugin_support::support_macros
      )
  else()
    set(_art_plugin_deps LIBRARIES CONDITIONAL
      art_Utilities
      fhiclcpp
      cetlib
      cetlib_except
      Boost::filesystem)
  endif()
  basic_plugin(${NAME} plugin ${ARGN} ${_art_plugin_deps})
  unset(_art_plugin_deps)
endmacro()

cmake_policy(POP)
