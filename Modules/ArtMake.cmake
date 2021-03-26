# art_make
#
# Identify the files in the current source directory and build
# libraries, dictionaries and plugins as appropriate.
#
####################################
# NOTES:
#
# Users may opt to just include art_make() in their CMakeLists.txt This
# implementation is intended to be called NO MORE THAN ONCE per
# subdirectory.
#
# * art_make() tries very hard to be intelligent, but it doesn't fit
# every need. In which case, you might need to call art_make_library(),
# art_make_exec(), cet_test() (CetTest.cmake), art_dictionary()
# (canvas_root_io/Modules/ArtDictionary.cmake) and/or build_plugin()
# (art/Modules/BuildPlugins.cmake) separately.
#
# * If art_make() doesn't quite fit your needs (different plugins of the
# same type have different library dependencies, for example), you can
# use art_make() with a suitable EXCLUDE option to do everything except
# your special case and then deal with it separately.
#
# * art_make() will not take care of the installation of headers into an
# installed product's include/ directory tree, or of files into the
# products source/ area. See install_headers() and install_source() in
# cetbuildtools/Modules/InstallSource.cmake.
#
# * art_make() knows about ROOT dictionaries (as signalled by the
# presence of classes.h and classes_def.xml), and the following plugin
# types:
#
#   * modules -- producers, filters, analyzers and outputs (*_module.cc)
#   * services (*_service.cc)
#   * sources (*_source.cc)
#
#  You may specify a plugin-type-specific library link list as
#  XXXX_LIBRARIES. If you have another plugin type (fleeble, say), you
#  may make its existence known to art_make() by specifying a (possibly
#  empty) FLEEBLE_LIBRARIES argument. Source files matching,
#  "*_fleeble.cc' will be identified by that command as being plugins of
#  type, "fleeble" and use FLEEBLE_LIBRARIES against which to link.
#
####################################
# USAGE:
#
# art_make( [LIBRARY_NAME <library name>]
#           [LIB_LIBRARIES <library list>]
#           [MODULE_LIBRARIES <library list>]
#           [SOURCE_LIBRARIES <library list>]
#           [SERVICE_LIBRARIES <library list>]
#           [DICT_LIBRARIES <library list>]
#           [DICT_COMPILE_FLAGS <flag list>]
#           [SUBDIRS <source subdirectory>] (e.g., detail)
#           [EXCLUDE <ignore these files>]
#           [WITH_STATIC_LIBRARY]
#           [BASENAME_ONLY] (passed to build_plugin)
#           [NO_PLUGINS]
#           [DICT_FUNCTIONS]
#           [USE_PRODUCT_NAME]
#         )
#
# * In art_make(), LIBRARIES has been REMOVED! use LIB_LIBRARIES instead.
#
# * If NONE of {MODULES,SOURCE,SERVICE,DICT}_LIBRARIES is specified,
# then LIB_LIBRARIES (or LIBRARIES) will be used as appropriate for
# plugins as appropriate. If ANY of the above are specified then
# libraries must be specified for every plugin type or dictionary
# encountered.
#
# * DICT_FUNCTIONS, if present, is passed to art_dictionary().
# * DICT_COMPILE_FLAGS, if present, are passed to art_dictionary().
#
# art_make_library( [LIBRARY_NAME <library name>]
#                   SOURCE <source code list>
#                   [LIBRARIES <library list>]
#                   [WITH_STATIC_LIBRARY]
#                   [NO_INSTALL]
#                   [USE_PRODUCT_NAME]
#                   [LIBRARY_NAME_VAR <var_name>] )
#
# * if USE_PRODUCT_NAME is specified, the product name will be prepended
#   to the calculated library name
# * USE_PRODUCT_NAME and LIBRARY_NAME are mutually exclusive
#
# * If LIBRARY_NAME_VAR is specified, then that variable will be set to
# contain the final name of the library.
#
# art_make_exec( NAME <executable name>
#                [SOURCE <source code list>]
#                [LIBRARIES <library link list>]
#                [USE_BOOST_UNIT]
#                [NO_INSTALL] )
#
####################################
#
# * See also ArtDictionary.cmake for art_dictionary() and
# BuildPlugins.cmake for build_plugin().
#
########################################################################

include_guard(DIRECTORY)

cmake_policy(PUSH)
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

include(BuildPlugins)
include(CetMake)
include(Compatibility)
include(CetTest)

####################################
# art_make_exec
####################################
macro(art_make_exec)
  warn_deprecated("art_make_exec()" NEW "cet_make_exec")
  cet_make_exec(${ARGV})
endmacro(art_make_exec)

####################################
# art_make_library
####################################
macro(art_make_library)
  set(_cet_aml_args "${ARGV}")
  if (ART_MAKE_PREPEND_PRODUCT_NAME)
    list(PREPEND _cet_aml_args USE_PROJECT_NAME)
  endif()
  if (_cet_aml_args MATCHES "(^|;)(NO_)?SOURCE(;|$)")
    cet_make_library(${_cet_aml_args})
  else()
    cet_make(LIB_ONLY ${_cet_aml_args})
  endif()
  unset(_cet_aml_args)
endmacro()

####################################
# art_make
####################################
function(art_make)
  set(flags BASENAME_ONLY LIB_ONLY NO_DICTIONARY NO_INSTALL NO_LIB NO_PLUGINS
    USE_PRODUCT_NAME USE_PROJECT_NAME)
  cet_regex_escape(VAR flags_regex ${flags})
  list(JOIN flags_regex "|" tmp)
  set(flags_regex "^(${tmp})$")
  set(seen_art_make_flags "${ARGV}")
  list(FILTER seen_art_make_flags INCLUDE REGEX "${flags_regex}")
  list(REMOVE_DUPLICATES seen_art_make_flags)
  list(TRANSFORM ARGV REPLACE "${flags_regex}" "NOP")
  if ("USE_PRODUCT_NAME" IN_LIST seen_art_make_flags AND
      "USE_PROJECT_NAME" IN_LIST seen_art_make_flags)
    message(WARNING "USE_PRODUCT_NAME and USE_PROJECT_NAME are synonymous")
    list(REMOVE_ITEM seen_art_make_flags "USE_PRODUCT_NAME")
  endif()
  foreach (flag IN LISTS seen_art_make_flags)
    set(AM_${flag} ${flag})
  endforeach()
  if (AM_LIB_ONLY AND AM_NO_LIB)
    # Nothing to do except hold up tea and no tea to confuse the GPP door
    # and gain access to the bridge of the Heart of Gold.
    message(FATAL_ERROR "art_make(): LIB_ONLY and NO_LIB are mutually exclusive")
  endif()
  set(flag_keywords_dict NO_CHECK_CLASS_VERSION NO_DEFAULT_LIBRARIES VERSION)
  set(flags_dict NO_CHECK_CLASS_VERSION NO_DEFAULT_LIBRARIES DICT_VERSION)
  set(one_arg_option_keywords_dict CLASSES_H CLASSES_DEF_XML DICT_NAME_VAR EXPORT_SET)
  set(one_arg_options_dict CLASSES_H CLASSES_DEF_XML DICT_NAME_VAR DICT_EXPORT_SET)
  set(list_option_keywords_dict COMPILE_FLAGS DICTIONARY_LIBRARIES REQUIRED_DICTIONARIES)
  set(list_options_dict DICT_COMPILE_FLAGS DICT_LIBRARIES REQUIRED_DICTIONARIES)
  set(flags "${flag_keywords_dict}")
  set(one_arg_options "${one_arg_option_keywords_dict}")
  set(list_options "${list_option_keywords_dict}")
  list(APPEND flags ${_cet_make_flags})
  list(APPEND one_arg_options ${_cet_make_one_arg_options})
  list(APPEND list_options ${_cet_make_list_options})

  # Identify caller-specified plugin types.
  set(default_plugin_types source module service tool) # Defaults.
  set(args "${ARGN}")
  list(FILTER args INCLUDE REGEX "^([A-Z]+)_(TYPE|LIBRARIES)$")
  list(TRANSFORM args REPLACE "_(TYPE|LIBRARIES)$" "")
  list(REMOVE_DUPLICATES args)
  list(TRANSFORM args TOLOWER OUTPUT_VARIABLE plugin_types)
  list(REMOVE_ITEM plugin_types dict lib ${default_plugin_types})
  list(PREPEND plugin_types ${default_plugin_types})
  list(TRANSFORM plugin_types REPLACE "^(.+)$" "*_\\1.cc"
    OUTPUT_VARIABLE plugin_glob)
  list(TRANSFORM plugin_types TOUPPER OUTPUT_VARIABLE plugin_prefixes)
  set(flags_plugin USE_BOOST_UNIT VERSION)
  set(one_arg_options_plugin EXPORT_SET LIB_TYPE SOVERSION)
  set(list_options_plugin LIBRARIES)
  foreach (option_type IN ITEMS flags one_arg_options list_options)
    foreach (plugin_prefix IN LISTS plugin_prefixes)
      list(TRANSFORM ${option_type}_plugin PREPEND "${plugin_prefix}_"
        OUTPUT_VARIABLE tmp)
      list(APPEND ${option_type} ${tmp})
    endforeach()
    list(REMOVE_DUPLICATES ${option_type})
  endforeach()
  # We have to parse everything at once and pass through to the right
  # place if we need to otherwise we could get confused with argument /
  # option boundaries with multiple parsing passes.
  cmake_parse_arguments(PARSE_ARGV 0 AM "${flags}" "${one_arg_options}" "${list_options}")
  # Identify plugin sources.
  set(plugins_glob ${plugin_glob})
  foreach(subdir IN LISTS AM_SUBDIRS)
    list(TRANSFORM plugin_glob PREPEND "${subdir}/"
      OUTPUT_VARIABLE tmp)
    list(APPEND plugins_glob ${tmp})
  endforeach()
  file(GLOB plugin_sources CONFIGURE_DEPENDS ${plugins_glob})
  cet_exclude_files_from(plugin_sources
    EXCLUDE ${AM_EXCLUDE} NOP REGEX "(^|/)[.#].*")
  # Exclude these files from consideration for cet_make() regardless of
  # whether we're making the plugin.
  cet_passthrough(APPEND KEYWORD EXCLUDE plugin_sources AM_EXCLUDE)
  set(flag_flag FLAG) # Flags are handled differently to the others.
  foreach (option_type IN ITEMS flag one_arg_option list_option)
    # Dictionaries.
    foreach (keyword opt IN ZIP_LISTS ${option_type}_keywords_dict ${option_type}s_dict)
      cet_passthrough(${flag_flag} APPEND KEYWORD ${keyword} AM_${opt} dict_args)
    endforeach()
    # Plugins
    foreach (plugin_type plugin_prefix IN ZIP_LISTS plugin_types plugin_prefixes)
      foreach (opt IN LISTS ${option_type}s_plugin)
        cet_passthrough(${flag_flag} APPEND KEYWORD ${opt} AM_${plugin_prefix}_${opt} ${plugin_type}_args)
      endforeach()
    endforeach()
    # For cet_make().
    foreach (opt IN LISTS _cet_make_${option_type}s)
      cet_passthrough(${flag_flag} APPEND AM_${opt} cet_make_args)
    endforeach()
    unset(flag_flag)
  endforeach()
  # Common options.
  foreach (type IN ITEMS dict LISTS plugin_types)
    string(TOUPPER "${type}" TYPE)
    cet_passthrough(FLAG APPEND AM_NO_INSTALL ${type}_args)
    if (NOT VERSION IN_LIST ${type}_args)
      cet_passthrough(FLAG APPEND AM_VERSION ${type}_args)
    endif()
    if (NOT (AM_NO_INSTALL OR EXPORT_SET IN_LIST ${type}_args))
      cet_passthrough(APPEND AM_EXPORT_SET ${type}_args)
    endif()
    cet_passthrough(FLAG APPEND AM_USE_PROJECT_NAME ${type}_args)
    if (NOT type STREQUAL "dict")
      cet_passthrough(FLAG APPEND AM_BASENAME_ONLY ${type}_args)
    endif()
  endforeach()

  ##################
  # Now actually put everything together.

  # Find sources for a library and make it.
  if (NOT AM_NO_LIB)
    # Retain historical behavior (BASENAME_ONLY was previously
    # applicable only to plugins) of a deprecated function (art_make).
    list(REMOVE_ITEM cet_make_args BASENAME_ONLY)
    art_make_library(${cet_make_args} LIBRARY_NAME_VAR library_name)
  endif()

  # Plugins.
  if (plugin_sources AND NOT (AM_LIB_ONLY OR AM_NO_PLUGINS))
    cet_regex_escape(VAR e_plugin_types ${plugin_types})
    list(JOIN e_plugin_types "|" plugin_regex)
    foreach (plugin_source IN LISTS plugin_sources)
      if (plugin_source MATCHES "^(.*/)?([^/]+)_(${plugin_regex})\.cc$")
        build_plugin("${CMAKE_MATCH_2}" "${CMAKE_MATCH_3}"
          ${${CMAKE_MATCH_3}_args} SOURCE ${plugin_source})
      endif()
    endforeach()
  endif()

  # Finish off with the dictionary.
  if (NOT (AM_LIB_ONLY OR AM_NO_DICTIONARY) AND
      EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/classes.h" AND
      EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/classes_def.xml")
    include(ArtDictionary)
    art_dictionary(${dict_args} DICTIONARY_LIBRARIES PRIVATE ${library_name})
  endif()
endfunction()

cmake_policy(POP)
