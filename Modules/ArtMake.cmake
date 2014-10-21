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
# art_make_exec(), art_make_test(), art_dictionary()
# (art/Modules/ArtDictionary.cmake) and/or simple_plugin()
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
#           [BASENAME_ONLY] (passed to simple_plugin)
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
# art_make_test( NAME <executable name>
#                [SOURCE <source code list>]
#                [LIBRARIES <library link list>]
#                [USE_BOOST_UNIT]
#                [INSTALL|NO_INSTALL]
#                [NO_AUTO] )
#
####################################
#
# * See also ArtDictionary.cmake for art_dictionary() and
# BuildPlugins.cmake for simple_plugin().
#
########################################################################

include(ArtDictionary)
include(CetMake)
include(CetParseArgs)
include(InstallSource)

macro (_debug_message)
  string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC)
  if (BTYPE_UC STREQUAL "DEBUG")
    message(STATUS "ART_MAKE: " ${ARGN})
  endif()
endmacro()

macro( _art_simple_plugin file type liblist )
  #message(STATUS "_art_simple_plugin: ${file} ${type} ${liblist}")
  STRING( REGEX REPLACE "^${CMAKE_CURRENT_SOURCE_DIR}/(.*)_${type}.cc" "\\1" plugbase "${file}" )
  #message(STATUS "_art_simple_plugin: have ${type} plugin ${plugbase}")
  #message(STATUS "_art_simple_plugin: AM_BASENAME_ONLY is ${AM_BASENAME_ONLY}")
  if (AM_USE_PRODUCT_NAME)
    set( plugbase ${product}_${plugbase} )
    _debug_message("Configured to build plugin ${plugbase} of type ${type} with USE_PRODUCT_NAME.")
    message(STATUS "_art_simple_plugin debug:  calculated plugin base name is now ${plugbase} for ${product}")
  endif()
  if( AM_BASENAME_ONLY )
    _debug_message("Configured to build plugin ${plugbase} of type ${type} with BASENAME_ONLY.")
    simple_plugin( ${plugbase} ${type} ${liblist} BASENAME_ONLY )
  else()
    _debug_message("Configured to build plugin ${plugbase} of type ${type}.")
    simple_plugin( ${plugbase} ${type} ${liblist} )
  endif()
endmacro( _art_simple_plugin )

####################################
# art_make_exec
####################################
macro( art_make_exec )
  set(cet_arguments ${ARGN})
  list(REMOVE_ITEM  cet_arguments NAME)
  cet_make_exec( ${cet_arguments} )
endmacro( art_make_exec )

####################################
# art_make_test
####################################
macro( art_make_test )
  cet_parse_args( AMT "" "INSTALL;NO_INSTALL;NO_AUTO" ${ARGN})
  list(REMOVE_ITEM ARGN INSTALL NO_INSTALL NO_AUTO)
  if (NOT AMT_INSTALL)
    list(APPEND ARGN NO_INSTALL)
  endif()
  cet_make_exec( ${ARGN} )
  if (NOT AMT_NO_AUTO)
    add_test(${AMT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${AMT_NAME})
  endif()
endmacro( art_make_test )

####################################
# art_make_library
####################################
function( art_make_library )
  cet_parse_args( AML "LIBRARY_NAME;LIBRARY_NAME_VAR;LIBRARIES;SOURCE" "USE_PRODUCT_NAME" ${ARGN})
  set(art_make_library_usage "USAGE: art_make_library( SOURCE <source code list> [LIBRARY_NAME <library name>] [LIBRARIES <library list>] [WITH_STATIC_LIBRARY] [NO_INSTALL] [USE_PRODUCT_NAME] [LIBRARY_NAME_VAR <var>])")

  # use either LIBRARY_NAME or USE_PRODUCT_NAME, not both
  if (AML_USE_PRODUCT_NAME AND AML_LIBRARY_NAME)
    message(FATAL_ERROR "ART_MAKE_LIBRARY: USE_PRODUCT_NAME and LIBRARY_NAME are mutually exclusive.")
  endif()

  # you must supply a source code list
  if( AML_SOURCE )
    if( AML_LIBRARY_NAME )
      set (art_make_lib_name ${AML_LIBRARY_NAME})
    else()
      # calculate base name
      if( DEFINED ENV{MRB_SOURCE} )
         STRING( REGEX REPLACE "^${CMAKE_SOURCE_DIR}/${product}/(.*)" "\\1" CURRENT_SUBDIR "${CMAKE_CURRENT_SOURCE_DIR}" )
         STRING( REGEX REPLACE "/" "_" art_make_lib_name "${CURRENT_SUBDIR}" )
      else()
         STRING( REGEX REPLACE "^${CMAKE_SOURCE_DIR}/(.*)" "\\1" CURRENT_SUBDIR "${CMAKE_CURRENT_SOURCE_DIR}" )
         STRING( REGEX REPLACE "/" "_" art_make_lib_name "${CURRENT_SUBDIR}" )
      endif()
      if (AML_USE_PRODUCT_NAME)
	set( art_make_lib_name ${product}_${art_make_lib_name} )
	message(STATUS "art_make_library debug:  calculated library name is now ${art_make_lib_name} for ${product}")
      endif()
    endif()
    if (AML_LIBRARIES)
      set(al LIBRARIES)
    endif()
    cet_make_library(LIBRARY_NAME ${art_make_lib_name}
      SOURCE ${AML_SOURCE}
      ${al} ${AML_LIBRARIES}
      ${AML_DEFAULT_ARGS}
      )
    if (AML_LIBRARY_NAME_VAR)
      set (${AML_LIBRARY_NAME_VAR} ${art_make_lib_name} PARENT_SCOPE)
    endif()
  else()
    message(${art_make_library_usage})
    message("art_make_library called from ${CMAKE_CURRENT_SOURCE_DIR}")
    message(FATAL_ERROR "ART_MAKE_LIBRARY: you must supply a source code list.")
  endif()
endfunction( art_make_library )

####################################
# art_make
####################################
function( art_make )
  set(arg_option_names
    LIBRARY_NAME LIBRARIES SUBDIRS EXCLUDE SOURCE LIB_LIBRARIES DICT_LIBRARIES DICT_COMPILE_FLAGS
    MODULE_LIBRARIES SERVICE_LIBRARIES SOURCE_LIBRARIES)
  set(plugin_types source module service) # Defaults
  # Add DICT_LIBRARIES, MODULE_LIBRARIES, GENERATOR_LIBRARIES, etc. as
  # appropriate.
  foreach (OPT ${ARGN})
    if ((NOT OPT STREQUAL "LIB_LIBRARIES") AND
        (NOT OPT STREQUAL "DICT_LIBRARIES") AND
        (NOT OPT STREQUAL "MODULE_LIBRARIES") AND
        (NOT OPT STREQUAL "SERVICE_LIBRARIES") AND
        (NOT OPT STREQUAL "SOURCE_LIBRARIES") AND
        (OPT MATCHES "^([A-Z]+)_LIBRARIES$"))
      string(TOLOWER ${CMAKE_MATCH_1} plugin_type)
      list(APPEND plugin_types ${plugin_type})
      list(APPEND arg_option_names ${OPT})
    endif()
  endforeach()
  foreach (plugin_type ${plugin_types})
    list(APPEND plugin_glob_list "*_${plugin_type}.cc")
  endforeach()
  set(art_file_list "")
  cet_parse_args( AM "${arg_option_names}" "WITH_STATIC_LIBRARY;BASENAME_ONLY;NO_PLUGINS;DICT_FUNCTIONS;USE_PRODUCT_NAME" ${ARGN})

  # has the cmake variable ART_MAKE_PREPEND_PRODUCT_NAME been specified?
  if( ART_MAKE_PREPEND_PRODUCT_NAME )
    set( AM_USE_PRODUCT_NAME TRUE )
  endif()

  if (AM_DICT_FUNCTIONS)
    set(AM_DICT_FUNCTIONS DICT_FUNCTIONS)
  else()
    unset(AM_DICT_FUNCTIONS)
  endif()
  if(AM_SOURCE)
    message(FATAL_ERROR "ART_MAKE: SOURCE is not a valid argument: library sources are computed.
Use EXCLUDE to exclude particular (eg exec) source files from library.")
  endif()

  if(AM_LIBRARIES)
    message(FATAL_ERROR "ART_MAKE: LIBRARIES is ambiguous -- use {LIB,DICT,SERVICE,MODULE,SOURCE,XXX}_LIBRARIES, instead.")
  endif()

  if (AM_USE_PRODUCT_NAME AND AM_LIBRARY_NAME)
    message(FATAL_ERROR "ART_MAKE: USE_PRODUCT_NAME and LIBRARY_NAME are mutually exclusive.")
  endif()
  if (AM_USE_PRODUCT_NAME AND AM_BASENAME_ONLY)
    message(FATAL_ERROR "ART_MAKE: USE_PRODUCT_NAME and BASENAME_ONLY are mutually exclusive.")
  endif()

  # check for extra link libraries
  if(AM_LIB_LIBRARIES)
    set(art_liblist ${AM_LIB_LIBRARIES})
  endif()

  # now look for other source files in this directory
  #message(STATUS "art_make debug: listed files ${art_file_list}")
  FILE( GLOB src_files *.c *.cc *.cpp *.C *.cxx )
  FILE( GLOB ignore_dot_files  .*.c .*.cc .*.cpp .*.C .*.cxx )
  FILE( GLOB plugin_files ${plugin_glob_list})
  # also check subdirectories
  if( AM_SUBDIRS )
    foreach( sub ${AM_SUBDIRS} )
      foreach (glob ${plugin_glob_list})
        list (APPEND subdir_plugin_glob_list ${sub}/${glob})
      endforeach()
	    FILE( GLOB subdir_src_files ${sub}/*.c ${sub}/*.cc ${sub}/*.cpp ${sub}/*.C ${sub}/*.cxx )
	    FILE( GLOB subdir_ignore_dot_files ${sub}/.*.c ${sub}/.*.cc ${sub}/.*.cpp ${sub}/.*.C ${sub}/.*.cxx )
	    FILE( GLOB subdir_plugin_files ${subdir_plugin_glob_list} )
      if (subdir_src_files)
	      list(APPEND src_files ${subdir_src_files})
      endif(subdir_src_files)
      if (subdir_ignore_dot_files)
	      list(APPEND ignore_dot_files ${subdir_ignore_dot_files})
      endif(subdir_ignore_dot_files)
      if (subdir_plugin_files)
	      list(APPEND plugin_files ${subdir_plugin_files})
      endif(subdir_plugin_files)
    endforeach(sub)
  endif( AM_SUBDIRS )
  if (ignore_dot_files OR plugin_files)
    LIST(REMOVE_ITEM src_files ${ignore_dot_files} ${plugin_files} )
  endif()
  if (ignore_dot_files)
    LIST(REMOVE_ITEM plugin_files ${ignore_dot_files})
  endif()
  #message(STATUS "art_make debug: exclude files ${AM_EXCLUDE}")
  if(AM_EXCLUDE)
    foreach( exclude_file ${AM_EXCLUDE} )
      LIST(REMOVE_ITEM src_files ${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file} )
      LIST(REMOVE_ITEM plugin_files ${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file} )
    endforeach( exclude_file )
  endif()
  #message(STATUS "art_make debug: other files ${src_files}")
  set(have_library FALSE)
  foreach( file ${src_files} )
    #message(STATUS "art_make debug: checking ${file}")
    set(have_file FALSE)
    foreach( known_file ${art_file_list} )
      if( "${file}" MATCHES "${known_file}" )
	      set(have_file TRUE)
	    endif()
    endforeach( known_file )
    if( NOT have_file )
      #message(STATUS "art_make debug: found new file ${file}")
      set(art_file_list ${art_file_list} ${file} )
      set(art_make_library_src ${art_make_library_src} ${file} )
      set(have_library TRUE)
    endif()
  endforeach(file)
  #message(STATUS "art_make debug: known files ${art_file_list}")
  if (AM_USE_PRODUCT_NAME)
    set(upn USE_PRODUCT_NAME)
  endif()

  if( have_library )
    if (AM_LIBRARY_NAME)
      set(ln LIBRARY_NAME)
    endif()
    if (AM_WITH_STATIC_LIBRARY)
      set(wsl WITH_STATIC_LIBRARY)
    endif()
    if (art_liblist)
      set(al LIBRARIES)
    endif()
    art_make_library(${wsl} ${upn} ${ln} ${AM_LIBRARY_NAME} ${al} ${art_liblist} SOURCE ${art_make_library_src}
      LIBRARY_NAME_VAR art_make_library_name
      )
    foreach(sfile ${art_make_library_src})
      get_filename_component(bn ${sfile} NAME)
      set(source_names ${source_names} " ${bn}")
    endforeach()

    _debug_message("Configured to build library ${art_make_library_name} with sources:
            ${source_names}.")
  endif( )

  # process plugin lists
  if( AM_NO_PLUGINS )
      _debug_message("Ignoring plugins in ${CMAKE_CURRENT_SOURCE_DIR}")
  else()
    foreach( plugin_file ${plugin_files} )
      if ("${plugin_file}" MATCHES "^.*_([a-z]+)\\.cc$")
        set (plugin_type ${CMAKE_MATCH_1})
        string (TOUPPER ${plugin_type} PLUGIN_TYPE)
      endif()
      _art_simple_plugin( ${plugin_file} ${plugin_type} "${AM_${PLUGIN_TYPE}_LIBRARIES}" )
    endforeach( plugin_file )
  endif( )

  # is there a dictionary?
  FILE(GLOB dictionary_header classes.h )
  FILE(GLOB dictionary_xml classes_def.xml )
  if( dictionary_header AND dictionary_xml )
    set(art_file_list ${art_file_list} ${dictionary_xml} ${dictionary_header} )
    if (have_library)
      set(art_make_dict_libraries ${art_make_library_name})
    endif()
    list(APPEND art_make_dict_libraries ${AM_DICT_LIBRARIES})
    if (AM_DICT_COMPILE_FLAGS)
      set(art_dictionary_flags COMPILE_FLAGS ${AM_DICT_COMPILE_FLAGS})
    endif()
    if(art_make_dict_libraries)
      art_dictionary( DICTIONARY_LIBRARIES ${AM_DICT_FUNCTIONS} ${art_make_dict_libraries} ${art_dictionary_flags} ${upn} DICT_NAME_VAR dictname)
    else()
      art_dictionary( ${AM_DICT_FUNCTIONS} ${art_dictionary_flags} ${upn} DICT_NAME_VAR dictname)
    endif()
    if (cet_generated_code) # Bubble up to top scope.
      set(cet_generated_code ${cet_generated_code} PARENT_SCOPE)
    endif()
    _debug_message("Configured to build dictionary ${dictname}.")
  endif()

endfunction( art_make )
