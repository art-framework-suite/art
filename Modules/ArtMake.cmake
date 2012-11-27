# art_make
#
# Identify the files in the current source directory and deal with them appropriately
# Users may opt to just include art_make() in their CMakeLists.txt
# This implementation is intended to be called NO MORE THAN ONCE per subdirectory.
#
# NOTE: art_make_exec, art_make_test_exec, cet_make_exec and cet_make_test_exec 
# are no longer part of cet_make or art_make.  
# cet_make_test_exec is redundant - use cet_make_exec.
#
# art_make( [LIBRARY_NAME <library name>]
#           [LIBRARIES <library list>]
#           [SUBDIRS <source subdirectory>] (e.g., detail)
#           [EXCLUDE <ignore these files>] )
#
#
# art_make_library( [LIBRARY_NAME <library name>]  
#                   [SOURCE <source code list>] 
#                   [LIBRARIES <library list>] 
#                   [WITH_STATIC_LIBRARY] )
#
# art_make_exec( NAME <executable name>  
#                [SOURCE <source code list>] 
#                [LIBRARIES <library link list>]
#                [USE_BOOST_UNIT]
#                [NO_INSTALL] )
# -- build a regular executable
#           [EXEC <exec source>]
#           [TEST <test source>]

include(CetMake)
include(CetParseArgs)
include(InstallSource)

macro( _art_simple_plugin file type liblist )
    #message(STATUS "_art_simple_plugin: ${file} ${type} ${liblist}")
    STRING( REGEX REPLACE "^${CMAKE_CURRENT_SOURCE_DIR}/(.*)_${type}.cc" "\\1" plugbase "${file}" )
    #message(STATUS "_art_simple_plugin: have ${type} plugin ${plugbase}")
    simple_plugin( ${plugbase} ${type} ${liblist} )
endmacro( _art_simple_plugin )

macro( art_make_library )
endmacro( art_make_library )

macro( art_make_exec )
  cet_make_exec( ${ARGN} )
endmacro( art_make_exec )

macro( art_make_test )
  cet_parse_args( AMT "NAME;LIBRARIES;SOURCE" "USE_BOOST_UNIT;NO_INSTALL" ${ARGN})
  cet_make_exec( ${ARGN} NO_INSTALL )
  ADD_TEST(${AMT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${AMT_NAME})
endmacro( art_make_test )

macro( art_make_library )
  cet_parse_args( AML "LIBRARY_NAME;LIBRARIES;EXEC;SOURCE" "WITH_STATIC_LIBRARY" ${ARGN})
  # calculate base name
  STRING( REGEX REPLACE "^${CMAKE_SOURCE_DIR}/(.*)" "\\1" CURRENT_SUBDIR "${CMAKE_CURRENT_SOURCE_DIR}" )
  STRING( REGEX REPLACE "/" "_" art_make_lib_name "${CURRENT_SUBDIR}" )
  if( AML_LIBRARY_NAME )
     cet_make_library( ${ARGN} )
  else()
     ##message(STATUS "ART_MAKE_LIBRARY: default library name ${art_make_lib_name}")
     cet_make_library( LIBRARY_NAME ${art_make_lib_name} ${ARGN} )
  endif()
endmacro( art_make_library )

macro( art_make )
  set(art_file_list "")
  set(art_liblist FALSE)
  set(art_make_usage "USAGE: art_make( [LIBRARY_NAME <library name>] [LIBRARIES <library list>] [EXEC <exec source>]  [TEST <test source>] [EXCLUDE <ignore these files>] )")
  #message(STATUS "art_make debug: called with ${ARGN} from ${CMAKE_CURRENT_SOURCE_DIR}")
  cet_parse_args( AM "LIBRARY_NAME;LIBRARIES;EXEC;SUBDIRS;TEST;EXCLUDE" "WITH_STATIC_LIBRARY" ${ARGN})

  # calculate base name
  STRING( REGEX REPLACE "^${CMAKE_SOURCE_DIR}/(.*)" "\\1" CURRENT_SUBDIR "${CMAKE_CURRENT_SOURCE_DIR}" )
  STRING( REGEX REPLACE "/" "_" art_make_name "${CURRENT_SUBDIR}" )
  ##if( AM_LIBRARY_NAME )
  ##   message(STATUS "ART_MAKE: specified library name ${AM_LIBRARY_NAME}")
  ##else()
  ##   message(STATUS "ART_MAKE: default library name ${art_make_lib_name}")
  ##endif()

  # check for extra link libraries
  if(AM_LIBRARIES)
     set(art_liblist ${AM_LIBRARIES})
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
    #message( STATUS "art_make debug: building library for ${CMAKE_CURRENT_SOURCE_DIR}")
    if(AM_LIBRARY_NAME)
      set(art_make_library_name ${AM_LIBRARY_NAME})
    else()
      set(art_make_library_name ${art_make_name})
    endif()
    if(AM_LIBRARIES) 
       link_libraries( ${art_liblist} )
    endif(AM_LIBRARIES) 
    #message( STATUS "art_make debug: calling add_library with ${art_make_library_name}  ${art_make_library_src}") 
    add_library( ${art_make_library_name} SHARED ${art_make_library_src} )
    install( TARGETS ${art_make_library_name} DESTINATION ${flavorqual_dir}/lib )
  else( )
    message( STATUS "art_make: no library for ${CMAKE_CURRENT_SOURCE_DIR}")
  endif( )

  # process plugin lists
  foreach( plugin_file ${plugin_sources} )
    _art_simple_plugin( ${plugin_file} "source" "${art_liblist}" )
  endforeach( plugin_file )
  foreach( plugin_file ${plugin_services} )
    _art_simple_plugin( ${plugin_file} "service" "${art_liblist}" )
  endforeach( plugin_file )
  foreach( plugin_file ${plugin_modules} )
    _art_simple_plugin( ${plugin_file} "module" "${art_liblist}" )
  endforeach( plugin_file )

  # is there a dictionary?
  FILE(GLOB dictionary_header classes.h )
  FILE(GLOB dictionary_xml classes_def.xml )
  if( dictionary_header AND dictionary_xml )
     #message( STATUS "art_make: found dictionary in ${CMAKE_CURRENT_SOURCE_DIR}")
     set(art_file_list ${art_file_list} ${dictionary_xml} ${dictionary_header} )
     if(AM_LIBRARIES) 
        build_dictionary( DICTIONARY_LIBRARIES ${art_liblist} )
     else()
        build_dictionary(  )
     endif()
  endif()

endmacro( art_make )
