#[================================================================[.rst:
X
-
#]================================================================]
include_guard()

cmake_minimum_required(VERSION 3.18.2...3.27 FATAL_ERROR)

include(BasicPlugin)

macro(art::source NAME)
  if (TARGET art_plugin_types::SourceT)
    # The vast majority of sources extant are based on the Source template.
    set(_art_source_deps LIBRARIES REG art_plugin_types::SourceT)
  elseif (TARGET art_plugin_types::Source)
    # The vast majority of sources extant are based on the Source template.
    set(_art_source_deps LIBRARIES REG art_plugin_types::Source)
  else()
    # Older art suites.
    set(_art_source_deps LIBRARIES CONDITIONAL
      art_Framework_IO_Sources
      art_Framework_Core
      art_Framework_Principal
      art_Persistency_Common
      art_Persistency_Provenance
      art_Utilities
      canvas
      fhiclcpp
      cetlib
      cetlib_except
      Boost::filesystem
    )
  endif()
  basic_plugin(${NAME} source NOP ${ARGN} ${_art_source_deps})
  unset(_art_source_deps)
endmacro()
