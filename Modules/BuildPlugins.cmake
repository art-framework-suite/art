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
# cetbuildtools/Modules/BasicPlugin.cmake ### MIGRATE-ACTION-REQUIRED: remove
# (https://cdcvs.fnal.gov/redmine/projects/cetbuildtools/repository/revisions/master/entry/Modules/BasicPlugin.cmake). ### MIGRATE-ACTION-REQUIRED: remove
########################################################################
include(BasicPlugin)

cmake_policy(PUSH)
cmake_policy(VERSION 3.3)

# simple plugin libraries
function(simple_plugin NAME TYPE)
  cmake_parse_arguments(PARSE_ARGV 2 SP "NOP" "LIB_TYPE" "")
  if (SP_LIB_TYPE AND NOT SP_LIB_TYPE MATCHES "^(MODULE|SHARED)$")
    message(FATAL_ERROR "simple_plugin: unsupported LIB_TYPE ${LIB_TYPE}")
  endif()
  if (TYPE STREQUAL "service")
    if (SP_LIB_TYPE STREQUAL "MODULE")
      message(WARNING "plugin type \"service\" does not support LIB_TYPE MODULE")
      set(SP_LIB_TYPE SHARED)
    endif()
    list(PREPEND SP_UNPARSED_ARGUMENTS
      LIBRARIES
      art_Framework_Services_Registry
      art_Persistency_Common
      art_Utilities
      canvas
      fhiclcpp
      cetlib
      cetlib_except
      Boost::filesystem NOP)
  elseif (TYPE STREQUAL "module" OR TYPE STREQUAL "source")
    list(PREPEND SP_UNPARSED_ARGUMENTS
      LIBRARIES
      art_Framework_Core
      art_Framework_Principal
      art_Persistency_Common
      art_Persistency_Provenance
      art_Utilities
      canvas
      fhiclcpp
      cetlib
      cetlib_except
      Boost::filesystem NOP)
  elseif (TYPE STREQUAL "tool")
    list(PREPEND SP_UNPARSED_ARGUMENTS
      LIBRARIES
      art_Utilities
      fhiclcpp
      cetlib
      cetlib_except
      Boost::filesystem NOP)
  endif()
  if (TYPE STREQUAL "source")
    list(PREPEND SP_UNPARSED_ARGUMENTS
      LIBRARIES
      art_Framework_IO_Sources
      Boost::filesystem NOP)
  endif()
  cet_passthrough(IN_PLACE SP_LIB_TYPE)
  basic_plugin(${NAME} ${TYPE} ${SP_LIB_TYPE} NOP ${SP_UNPARSED_ARGUMENTS})
endfunction()

cmake_policy(POP)
