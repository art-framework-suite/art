#[================================================================[.rst:
X
=
#]================================================================]
include_guard()

cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

include(BasicPlugin)

macro(art::module NAME)
  if (TARGET art_plugin_types::module)
    set(_art_module_deps LIBRARIES REG art_plugin_types::module)
  else()
    set(_art_module_deps LIBRARIES CONDITIONAL
      art_Framework_Core
      art_Framework_Principal
      art_Framework_Services_Registry
      art_Persistency_Common
      art_Persistency_Provenance
      art_Utilities
      canvas
      fhiclcpp
      cetlib
      cetlib_except
      ROOT::Core
      Boost::filesystem
    )
  endif()
  basic_plugin(${NAME} module ${ARGN} ${_art_module_deps})
  unset(_art_module_deps)
endmacro()
