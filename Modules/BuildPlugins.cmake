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

find_package(Boost REQUIRED COMPONENTS filesystem system)

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
  basic_plugin(${name} ${type} NOP ${simple_plugin_liblist} ${ARGN} SOURCE ${SP_SOURCE})
endfunction(simple_plugin name type)

cmake_policy(POP)
