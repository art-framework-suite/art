# macros for building plugin libraries
#
# The plugin type is expected to be service, source, or module, but we
# do not enforce this in order to allow for user- or experiment-defined
# plugins.
#
# USAGE:
#
# simple_plugin( <name> <plugin type> [<basic_plugin options>]
#                [[NOP] <library list>] )
#
# Options:
#
# NOP
#
#    Dummy option for the purpose of separating (say) multi-option
#    arguments from non-option arguments.
#
# For other available options, please see
# cetbuildtools/Modules/BasicPlugin.cmake
# (https://cdcvs.fnal.gov/redmine/projects/cetbuildtools/repository/revisions/master/entry/Modules/BasicPlugin.cmake).
########################################################################
include(BasicPlugin)

cmake_policy(PUSH)
cmake_policy(VERSION 3.3)

macro (_sp_debug_message)
  string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC)
  if (BTYPE_UC STREQUAL "DEBUG")
    message(STATUS "SIMPLE_PLUGIN: " ${ARGN})
  endif()
endmacro()

# simple plugin libraries
function(simple_plugin name type)
  cmake_parse_arguments(SP "NOP" "" "SOURCE" ${ARGN})
  if(NOT simple_plugin_liblist)
    set(simple_plugin_liblist)
  endif()
  if("${type}" STREQUAL "service")
    list(INSERT simple_plugin_liblist 0
      art_Framework_Services_Registry
      art_Persistency_Common
      art_Utilities
      canvas
      fhiclcpp
      cetlib
      cetlib_except
      ${Boost_FILESYSTEM_LIBRARY}
      ${Boost_SYSTEM_LIBRARY}
      )
  elseif("${type}" STREQUAL "module" OR "${type}" STREQUAL "source")
    list(INSERT simple_plugin_liblist 0
      art_Framework_Core
      art_Framework_Principal
      art_Persistency_Common
      art_Persistency_Provenance
      art_Utilities
      canvas
      fhiclcpp
      cetlib
      cetlib_except
      ${ROOT_Core_LIBRARY}
      ${Boost_FILESYSTEM_LIBRARY}
      ${Boost_SYSTEM_LIBRARY}
      )
  elseif("${type}" STREQUAL "tool")
    list(INSERT simple_plugin_liblist 0
      art_Utilities
      fhiclcpp
      cetlib
      cetlib_except
      ${Boost_FILESYSTEM_LIBRARY}
      ${Boost_SYSTEM_LIBRARY}
      )
  endif()
  if ("${type}" STREQUAL "source")
    list(INSERT simple_plugin_liblist 0
      art_Framework_IO_Sources
      ${Boost_FILESYSTEM_LIBRARY}
      ${Boost_SYSTEM_LIBRARY}
      )
  endif()
  check_ups_version(cetbuildtools ${cetbuildtools_UPS_VERSION} v4_05_00 PRODUCT_MATCHES_VAR BP_HAS_SOURCE)
  if(SP_SOURCE)
    if (BP_HAS_SOURCE)
      list(INSERT SP_SOURCE 0 SOURCE)
    else()
      message(FATAL_ERROR "SOURCE option specified, but not supported by cetbuildtools ${CETBUILDTOOLS_VERSION}")
    endif()
  endif()
  check_ups_version(cetbuildtools ${cetbuildtools_UPS_VERSION} v4_06_00 PRODUCT_MATCHES_VAR BP_HAS_NOP)
  if (BP_HAS_NOP AND NOT SP_NOP)
    # Set it anyway so we have a good separator.
    set(SP_NOP TRUE)
  elseif(SP_NOP AND NOT BP_HAS_NOP)
    # Not a problem, it's already done its job.
    unset(SP_NOP)
  endif()
  if (SP_NOP)
    set (NOP_ARG NOP)
  endif()
  basic_plugin(${name} ${type} ${NOP_ARG} ${simple_plugin_liblist} ${ARGN} ${SP_SOURCE})
endfunction(simple_plugin name type)

cmake_policy(POP)
