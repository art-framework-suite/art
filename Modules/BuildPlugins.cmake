# macros for building plugin libraries
#
# The plugin type is expected to be service, source, or module,
# but we do not enforce this.
#
# USAGE:
# simple_plugin( <name> <plugin type>
#                [library list]
#                [ALLOW_UNDERSCORES]
#                [NOINSTALL] )
#        the base plugin name is derived from the current source code subdirectory
#        specify NOINSTALL when building a plugin for the tests

# simple plugin libraries
include(CetParseArgs)
macro (simple_plugin name type)
  cet_parse_args(SP "" "USE_BOOST_UNIT;ALLOW_UNDERSCORES;NOINSTALL" ${ARGN})
  #message( STATUS "simple_plugin: PACKAGE_TOP_DIRECTORY is ${PACKAGE_TOP_DIRECTORY}")
  # base name on current subdirectory
  if( PACKAGE_TOP_DIRECTORY )
     STRING( REGEX REPLACE "^${PACKAGE_TOP_DIRECTORY}/(.*)" "\\1" CURRENT_SUBDIR "${CMAKE_CURRENT_SOURCE_DIR}" )
  else()
     STRING( REGEX REPLACE "^${CMAKE_SOURCE_DIR}/(.*)" "\\1" CURRENT_SUBDIR "${CMAKE_CURRENT_SOURCE_DIR}" )
  endif()
  if( NOT SP_ALLOW_UNDERSCORES )
    string(REGEX MATCH [_] has_underscore "${CURRENT_SUBDIR}")
    if( has_underscore )
      message(SEND_ERROR  "found underscore in plugin subdirectory: ${CURRENT_SUBDIR}" )
    endif( has_underscore )
    string(REGEX MATCH [_] has_underscore "${name}")
    if( has_underscore )
      message(SEND_ERROR  "found underscore in plugin name: ${name}" )
    endif( has_underscore )
  endif()
  STRING( REGEX REPLACE "/" "_" plugname "${CURRENT_SUBDIR}" )
  set(plugin_name "${plugname}_${name}_${type}")
  set(codename "${name}_${type}.cc")
  #message(STATUS "SIMPLE_PLUGIN: generating ${plugin_name}")
  add_library(${plugin_name} SHARED ${codename} )
  set(simple_plugin_liblist "${SP_DEFAULT_ARGS}")
  if(SP_USE_BOOST_UNIT)
    set_target_properties(${plugin_name}
      PROPERTIES
      COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
      COMPILE_FLAGS -Wno-overloaded-virtual
      )
    list(INSERT simple_plugin_liblist 0 ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
  endif()
  if("${type}" STREQUAL "service")
    list(INSERT simple_plugin_liblist 0 art_Framework_Services_Registry)
  elseif("${type}" STREQUAL "module" OR "${type}" STREQUAL "source")
    list(INSERT simple_plugin_liblist 0 art_Framework_Core)
  endif()
  if( simple_plugin_liblist )
    target_link_libraries( ${plugin_name} ${simple_plugin_liblist} )
  endif( simple_plugin_liblist )
  if( NOT SP_NOINSTALL )
    install( TARGETS ${plugin_name}  DESTINATION ${flavorqual_dir}/lib )
  endif()
endmacro (simple_plugin name type)
