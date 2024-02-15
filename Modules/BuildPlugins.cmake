#[================================================================[.rst:
X
-
#]================================================================]
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
include_guard()

cmake_minimum_required(VERSION 3.18.2...3.27 FATAL_ERROR)

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

find_package(messagefacility QUIET)
if (messagefacility_FOUND)
  include(MessagefacilityPlugins)
endif()
find_package(art QUIET)
if (art_FOUND)
  include(ArtPlugins)
endif()

# Simple plugin libraries - art suite packages are found automatically.
macro(simple_plugin NAME SUFFIX)
  foreach (_sp_pkg _sp_tgt _sp_var IN ZIP_LISTS
      _simple_plugin_pkg_list
      _simple_plugin_target_list
      _simple_plugin_var_list)
    if (NOT (TARGET ${_sp_tgt} OR _sp_var))
      find_package(${_sp_pkg} QUIET REQUIRED)
    endif()
  endforeach()
  unset(_sp_pkg)
  unset(_sp_tgt)
  unset(_sp_var)
  build_plugin(${ARGV})
endmacro()

# Per simple_plugin() without the overhead of finding packages one may
# not need (historical).
macro(build_plugin NAME SUFFIX)
  _get_plugin_base(_bp_base _bp_args ${ARGN})
  if ("${_bp_base}" STREQUAL "")
    set(_bp_base ${SUFFIX})
  endif()
  if (_bp_base MATCHES "^[A-Z]")
    string(PREPEND _bp_base "art::")
  else()
    string(REGEX REPLACE "^(module|plugin|service|source|tool)$"
      "art::\\1"
      _bp_base
      "${_bp_base}")
  endif()
  string(JOIN " " _bp_arg_string ${NAME} ${_bp_base} ${_bp_args})
  warn_deprecated(build_plugin NEW "cet_build_plugin(${_bp_arg_string})"
    SINCE "cetmodules 2.25.00")
  cet_build_plugin(${NAME} ${_bp_base} ${_bp_args})
  unset(_bp_base)
  unset(_bp_args)
  unset(_bp_arg_string)
endmacro()

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
