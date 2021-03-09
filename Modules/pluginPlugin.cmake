include_guard(DIRECTORY)

cmake_policy(PUSH)
cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

include(BasicPlugin)

set(_plugin_plugin_SUFFIX plugin)

function(ResultsProducer NAME)
  if (TARGET art_plugin_types::ResultsProducer)
    basic_plugin(${NAME} ${_plugin_plugin_SUFFIX}
      ${ARGN} LIBRARIES REG art_plugin_types::ResultsProducer)
  else()
    message(FATAL_ERROR "attempt to build a plugin type ${_plugin_plugin_SUFFIX} not currently known")
  endif()
endfunction()

function(plugin_plugin NAME BASE)
  if (BASE MATCHES "Plugin\$" AND TARGET art_plugin_types::${BASE})
    set(deps REG art_plugin_types::${BASE})
  elseif (BASE)
    message(FATAL_ERROR "unknown BASE type ${BASE} for plugin type ${_plugin_plugin_SUFFIX}
Define CMake function ${BASE}(NAME) or variable ${BASE}_LIBRARIES before calling build_plugin()")
  elseif (TARGET art_plugin_support::plugin_config_macros)
    set(deps REG
      art_plugin_support::plugin_config_macros
      art_plugin_support::support_macros
    )
  else()
    set(deps CONDITIONAL
      art_Utilities
      fhiclcpp
      cetlib
      cetlib_except
      Boost::filesystem)
  endif()
  basic_plugin(${NAME} ${_plugin_plugin_SUFFIX} ${ARGN}
    LIBRARIES ${deps})
endfunction()

cmake_policy(POP)
