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
# cetbuildtools/Modules/BasicPlugin.cmake ### MIGRATE-NO-ACTION
# (https://cdcvs.fnal.gov/redmine/projects/cetbuildtools/repository/revisions/master/entry/Modules/BasicPlugin.cmake). ### MIGRATE-NO-ACTION
########################################################################
include_guard(DIRECTORY)

cmake_policy(PUSH)
cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

include(BasicPlugin)

set(_simple_plugin_pkg_list cetlib_except hep_concurrency cetlib fhiclcpp
  messagefacility canvas art canvas_root_io art_root_io ROOT)
set(_simple_plugin_target_list cetlib_except::cetlib_except hep_concurrency::hep_concurrency
  cetlib::cetlib fhiclcpp::fhiclcpp messagefacility::MF_MessageLogger
  canvas::canvas art::Framework_Core canvas_root_io::canvas_root_io
  art_root_io::art_root_io ROOT::Core)
set(_simple_plugin_var_list CETLIB_EXCEPT HEP_CONCURRENCY CETLIB FHICLCPP
  MF_MESSAGELOGGER CANVAS ART_FRAMEWORK_CORE CANVAS_ROOT_IO ART_ROOT_IO
  ROOT_CORE_LIBRARY)

cet_find_package(messagefacility PRIVATE QUIET)
include(mfPlugin OPTIONAL)
include(mfStatsPlugin OPTIONAL)
include(modulePlugin)
include(pluginPlugin)
include(servicePlugin)
include(sourcePlugin)
include(toolPlugin)

# Simple plugin libraries - art suite packages are found automatically.
function(simple_plugin)
  foreach (pkg tgt var IN ZIP_LISTS
      _simple_plugin_pkg_list _simple_plugin_target_list _simple_plugin_var_list)
    if (NOT (TARGET ${tgt} OR var))
      cet_find_package(${pkg} PRIVATE QUIET REQUIRED)
    endif()
  endforeach()
  build_plugin(${ARGV})
endfunction()

# Per simple_plugin() without the overhead of finding packages one may
# not need.
function(build_plugin NAME SUFFIX)
  set(liblist)
  _get_plugin_base(BASE ARGN ${ARGN})
  if (BASE AND COMMAND "${BASE}")
    cmake_language(CALL "${BASE}" ${NAME} ${ARGN})
    return()
  elseif (BASE AND ${BASE}_LIBRARIES)
    set(liblist "${${BASE}_LIBRARIES}")
  elseif (COMMAND "${SUFFIX}")
    cmake_language(CALL "${SUFFIX}" ${NAME} ${ARGN})
    return()
  elseif (COMMAND "${SUFFIX}_plugin")
    cmake_language(CALL "${SUFFIX}_plugin" ${NAME} "${BASE}" ${ARGN})
    return()
  elseif (${SUFFIX}_LIBRARIES)
    set(liblist "${${SUFFIX}_LIBRARIES}")
  endif()
  basic_plugin(${NAME} ${SUFFIX} ${liblist} NOP ${ARGN})
endfunction()

function(_get_plugin_base RESULT_VAR REMAINDER_VAR)
  set(result)
  list(FIND ARGN "BASE" idx)
  if (idx GREATER -1)
    list(REMOVE_AT ARGN ${idx})
    list(GET ARGN ${idx} result)
    if (NOT result MATCHES "^(${_e_bp_args})$")
      list(REMOVE_AT ARGN ${idx})
    else()
      unset(result)
    endif()
  endif()
  set(${RESULT_VAR} ${result} PARENT_SCOPE)
  set(${REMAINDER_VAR} ${ARGN} PARENT_SCOPE)
endfunction()

cmake_policy(POP)
