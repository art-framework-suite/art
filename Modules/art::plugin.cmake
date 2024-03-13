#[================================================================[.rst:
X
-
#]================================================================]
include_guard()

cmake_minimum_required(VERSION 3.18.2...3.27 FATAL_ERROR)

include(BasicPlugin)

macro(art::plugin NAME)
  cmake_parse_arguments(_ap "" "SUFFIX" "" ${ARGN})
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
  if ("${_ap_SUFFIX}" STREQUAL "")
    set(_ap_SUFFIX plugin)
  endif()
  basic_plugin(${NAME} ${_ap_SUFFIX} ${_ap_UNPARSED_ARGUMENTS} ${_art_plugin_deps})
  unset(_art_plugin_deps)
endmacro()
