include_guard(DIRECTORY)

cmake_policy(PUSH)
cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

include(BasicPlugin)

set(_tool_plugin_SUFFIX tool)

function(tool_plugin NAME BASE)
  if (BASE MATCHES "Tool\$" AND TARGET art_plugin_types::${BASE})
    set(deps REG art_plugin_types::${BASE})
  elseif (BASE)
    message(FATAL_ERROR "unknown BASE type ${BASE} for plugin type ${_tool_plugin_SUFFIX}
Define CMake function ${BASE}(NAME) or variable ${BASE}_LIBRARIES before calling build_plugin()")
  elseif (TARGET art_plugin_types::${_tool_plugin_SUFFIX})
    set(deps REG art_plugin_types::${_tool_plugin_SUFFIX})
  else()
    # Older art suites.
    set(deps CONDITIONAL
      art_Utilities
      fhiclcpp
      cetlib
      cetlib_except
      Boost::filesystem
    )
  endif()
  basic_plugin(${NAME} ${_tool_plugin_SUFFIX} ${ARGN}
    LIBRARIES ${deps})
endfunction()

cmake_policy(POP)
