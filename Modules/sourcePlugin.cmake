include_guard(DIRECTORY)

cmake_policy(PUSH)
cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

include(BasicPlugin)

set(_source_plugin_SUFFIX source)

function(DRISISource NAME)
  basic_plugin(${NAME} ${_source_plugin_SUFFIX}
    ${ARGN} LIBRARIES
    CONDITIONAL art::Framework_Core
    REG art_plugin_types::InputSource)
endfunction()

function(source_plugin NAME BASE)
  if (NOT BASE AND TARGET art_plugin_types::Source)
    # The vast majority of sources extant are based on the Source template.
    set(deps REG art_plugin_types::Source)
  elseif (BASE MATCHES "Source\$" AND TARGET art_plugin_types::${BASE})
    set(deps REG art_plugin_types::${BASE})
  elseif (BASE)
    message(FATAL_ERROR "unknown BASE type ${BASE} for plugin type ${_source_plugin_SUFFIX}
Define CMake function ${BASE}(NAME) or variable ${BASE}_LIBRARIES before calling build_plugin()")
  else()
    # Older art suites.
    set(deps CONDITIONAL
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
  basic_plugin(${NAME} ${_source_plugin_SUFFIX} NOP ${ARGN}
    LIBRARIES REG ${deps})
endfunction()

cmake_policy(POP)
