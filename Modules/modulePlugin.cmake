include_guard(DIRECTORY)

cmake_policy(PUSH)
cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

include(BasicPlugin)

set(_module_plugin_SUFFIX module)

function(module_plugin NAME BASE)
  if (BASE MATCHES "(Analyzer|Filter|Module|Output|Producer)\$" AND
      TARGET art_plugin_types::${BASE})
    set(deps REG art_plugin_types::${BASE})
  elseif (BASE)
    message(FATAL_ERROR "unknown BASE type ${BASE} for plugin type ${_module_plugin_SUFFIX}
Define CMake function ${BASE}(NAME) or variable ${BASE}_LIBRARIES before calling build_plugin()")
  elseif (TARGET art_plugin_types::${_module_plugin_SUFFIX})
    set(deps REG art_plugin_types::${_module_plugin_SUFFIX})
  else()
    set(deps CONDITIONAL
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
      ${ROOT_Core_LIBRARY}
      Boost::filesystem
    )
  endif()
  basic_plugin(${NAME} ${_module_plugin_SUFFIX} ${ARGN}
    LIBRARIES ${deps})
endfunction()

cmake_policy(POP)
