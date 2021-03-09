include_guard(DIRECTORY)

cmake_policy(PUSH)
cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

include(BasicPlugin)

set(_service_plugin_SUFFIX service)

function(systemService NAME)
  if (TARGET art_plugin_types::systemService)
    basic_plugin(${NAME} ${_service_plugin_SUFFIX}
      ${ARGN} LIBRARIES
      CONDITIONAL art_plugin_types::serviceDeclaration
      REG art_plugin_types::systemService)
  else()
    message(FATAL_ERROR "attempt to build an unknown BASE type systemService for plugin type ${_service_plugin_SUFFIX}")
  endif()
endfunction()

function(service_plugin NAME BASE)
  if (BASE MATCHES "Service\$" AND TARGET art_plugin_types::${BASE})
    set(deps CONDITIONAL art_plugin_types::${BASE}
      REG art_plugin_types::serviceDefinition)
  elseif (BASE)
    message(FATAL_ERROR "unknown BASE type ${BASE} for plugin type ${_service_plugin_SUFFIX}
Define CMake function ${BASE}(NAME) or variable ${BASE}_LIBRARIES before calling build_plugin()")
  elseif (TARGET art_plugin_types::serviceDeclaration)
    set(deps CONDITIONAL art_plugin_types::serviceDeclaration
      REG art_plugin_types::serviceDefinition)
  else()
    # Older art suites.
    set(deps PUBLIC
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
  basic_plugin(${NAME} ${_service_plugin_SUFFIX} ${ARGN}
    LIBRARIES ${deps})
endfunction()

cmake_policy(POP)
