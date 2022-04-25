#[================================================================[.rst:
X
=
#]================================================================]
include_guard()

cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

include(BasicPlugin)

macro(art::service NAME)
  if (TARGET art_plugin_types::serviceDeclaration)
    set(_art_service_deps LIBRARIES CONDITIONAL art_plugin_types::serviceDeclaration
      REG art_plugin_types::serviceDefinition)
  else()
    # Older art suites.
    set(_art_service_deps LIBRARIES PUBLIC
      art_Framework_Services_Registry
      art_Persistency_Common
      art_Utilities
      canvas
      fhiclcpp
      cetlib
      cetlib_except
      Boost::filesystem
    )
  endif()
  basic_plugin(${NAME} service ${ARGN} ${_art_service_deps})
  unset(_art_service_deps)
endmacro()
