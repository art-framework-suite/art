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
####################################
# USAGE:
#
# art_make( [LIBRARY_NAME <library name>]
#           [LIB_LIBRARIES <library list>]
#           [MODULE_LIBRARIES <library list>]
#           [SOURCE_LIBRARIES <library list>]
#           [SERVICE_LIBRARIES <library list>]
#           [DICT_LIBRARIES <library list>]
#           [SUBDIRS <source subdirectory>] (e.g., detail)
#           [EXCLUDE <ignore these files>]
#           [WITH_STATIC_LIBRARY]
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
# art_make_library( [LIBRARY_NAME <library name>]  
#                   SOURCE <source code list>
#                   [LIBRARIES <library list>] 
#                   [WITH_STATIC_LIBRARY]
#                   [NO_INSTALL]
#                   [LIBRARY_NAME_VAR <var_name>] )
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
  _debug_message("Configured to build plugin ${plugbase} of type ${type}.")
  simple_plugin( ${plugbase} ${type} ${liblist} )
endmacro( _art_simple_plugin )

####################################
# art_make_exec
####################################
macro( art_make_exec )
  cet_make_exec( ${ARGN} )
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
  cet_parse_args( AML "LIBRARY_NAME;LIBRARY_NAME_VAR;LIBRARIES;SOURCE" "" ${ARGN})
  set(art_make_library_usage "USAGE: art_make_library( SOURCE <source code list> [LIBRARY_NAME <library name>] [LIBRARIES <library list>] [WITH_STATIC_LIBRARY] [NO_INSTALL] [LIBRARY_NAME_VAR <var>])")
  # you must supply a source code list
  if( AML_SOURCE )
    if( AML_LIBRARY_NAME )
      set (art_make_lib_name ${AML_LIBRARY_NAME})
    else()
      # calculate base name
      STRING( REGEX REPLACE "^${CMAKE_SOURCE_DIR}/(.*)" "\\1" CURRENT_SUBDIR "${CMAKE_CURRENT_SOURCE_DIR}" )
      STRING( REGEX REPLACE "/" "_" art_make_lib_name "${CURRENT_SUBDIR}" )
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
  set(art_file_list "")
  cet_parse_args( AM "LIBRARY_NAME;LIBRARIES;LIB_LIBRARIES;DICT_LIBRARIES;SERVICE_LIBRARIES;MODULE_LIBRARIES;SOURCE_LIBRARIES;SUBDIRS;EXCLUDE" "WITH_STATIC_LIBRARY" ${ARGN})

  if(AM_LIBRARIES)
    message(FATAL_ERROR "ART_MAKE: LIBRARIES is ambiguous -- use {LIB,DICT,SERVICE,MODULE,SOURCE}_LIBRARIES, instead.")
    if(AM_DICT_LIBRARIES OR AM_SERVICE_LIBRARIES OR AM_MODULE_LIBRARIES OR AM_SOURCE_LIBRARIES)
      set(ignore_libraries YES)
    endif()
  endif()

  # check for extra link libraries
  if(AM_LIB_LIBRARIES)
    set(art_liblist ${AM_LIB_LIBRARIES})
  endif()

  # now look for other source files in this directory
  #message(STATUS "art_make debug: listed files ${art_file_list}")
  FILE( GLOB src_files *.c *.cc *.cpp *.C *.cxx )
  FILE( GLOB plugin_sources  *_source.cc )
  FILE( GLOB plugin_services *_service.cc )
  FILE( GLOB plugin_modules  *_module.cc )
  # also check subdirectories
  if( AM_SUBDIRS )
    foreach( sub ${AM_SUBDIRS} )
	    FILE( GLOB subdir_src_files ${sub}/*.c ${sub}/*.cc ${sub}/*.cpp ${sub}/*.C ${sub}/*.cxx )
	    FILE( GLOB subdir_plugin_sources  ${sub}/*_source.cc )
	    FILE( GLOB subdir_plugin_services ${sub}/*_service.cc )
	    FILE( GLOB subdir_plugin_modules  ${sub}/*_module.cc )
      if( subdir_src_files )
	      list(APPEND  src_files ${subdir_src_files})
      endif( subdir_src_files )
      if( subdir_plugin_sources )
	      list(APPEND  plugin_sources ${subdir_plugin_sources})
      endif( subdir_plugin_sources )
      if( subdir_plugin_services )
	      list(APPEND  plugin_services ${subdir_plugin_services})
      endif( subdir_plugin_services )
      if( subdir_plugin_modules )
	      list(APPEND  plugin_modules ${subdir_plugin_modules})
      endif( subdir_plugin_modules )
    endforeach(sub)
  endif( AM_SUBDIRS )
  if( plugin_sources OR plugin_services OR plugin_modules )
    #message(STATUS "plugin sources ${plugin_sources}")
    #message(STATUS "plugin services ${plugin_services}")
    #message(STATUS "plugin modules ${plugin_modules}" )
    LIST( REMOVE_ITEM src_files ${plugin_sources} ${plugin_services} ${plugin_modules} )
  endif()
  #message(STATUS "art_make debug: exclude files ${AM_EXCLUDE}")
  if(AM_EXCLUDE)
    foreach( exclude_file ${AM_EXCLUDE} )
      LIST( REMOVE_ITEM src_files ${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file} )
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
    art_make_library(${wsl} ${ln} ${AM_LIBRARY_NAME} ${al} ${art_liblist} SOURCE ${art_make_library_src}
      LIBRARY_NAME_VAR art_make_library_name
      )
    foreach(sfile ${art_make_library_src})
      get_filename_component(bn ${sfile} NAME)
      set(source_names ${source_names} " ${bn}")
    endforeach()

    _debug_message("Configured to build library ${art_make_library_name} with sources:
            ${source_names}.")
  endif( )

  # Process extra library lists.
  if (NOT ignore_libraries)
    if (NOT AM_SOURCE_LIBRARIES)
      set(AM_SOURCE_LIBRARIES ${art_liblist})
    endif()
    if (NOT AM_SERVICE_LIBRARIES)
      set(AM_SERVICE_LIBRARIES ${art_liblist})
    endif()
    if (NOT AM_MODULE_LIBRARIES)
      set(AM_MODULE_LIBRARIES ${art_liblist})
    endif()
    if (NOT AM_DICT_LIBRARIES)
      set(AM_DICT_LIBRARIES ${art_liblist})
    endif()
  endif()

  # process plugin lists
  foreach( plugin_file ${plugin_sources} )
    _art_simple_plugin( ${plugin_file} "source" "${AM_SOURCE_LIBRARIES}" )
  endforeach( plugin_file )
  foreach( plugin_file ${plugin_services} )
    _art_simple_plugin( ${plugin_file} "service" "${AM_SERVICE_LIBRARIES}" )
  endforeach( plugin_file )
  foreach( plugin_file ${plugin_modules} )
    _art_simple_plugin( ${plugin_file} "module" "${AM_MODULE_LIBRARIES}" )
  endforeach( plugin_file )

  # is there a dictionary?
  FILE(GLOB dictionary_header classes.h )
  FILE(GLOB dictionary_xml classes_def.xml )
  if( dictionary_header AND dictionary_xml )
    set(art_file_list ${art_file_list} ${dictionary_xml} ${dictionary_header} )
    if (have_library)
      set(art_make_dict_libraries ${art_make_library_name})
    endif()
    list(APPEND art_make_dict_libraries ${AM_DICT_LIBRARIES})
    if(art_make_dict_libraries)
      art_dictionary( DICTIONARY_LIBRARIES ${art_make_dict_libraries} DICT_NAME_VAR dictname)
    else()
      art_dictionary( DICT_NAME_VAR dictname)
    endif()
    _debug_message("Configured to build dictionary ${dictname}.")
  endif()

endfunction( art_make )
