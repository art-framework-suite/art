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
  cet_parse_args(SP "" "ALLOW_UNDERSCORES;NOINSTALL" ${ARGN})
  #message( STATUS "simple_plugin: PACKAGE_TOP_DIRECTORY is ${PACKAGE_TOP_DIRECTORY}")
  # base name on current subdirectory
  if( PACKAGE_TOP_DIRECTORY )
     STRING( REGEX REPLACE "^${PACKAGE_TOP_DIRECTORY}/(.*)" "\\1" CURRENT_SUBDIR "${CMAKE_CURRENT_SOURCE_DIR}" )
  else()
     STRING( REGEX REPLACE "^${CMAKE_SOURCE_DIR}/(.*)" "\\1" CURRENT_SUBDIR "${CMAKE_CURRENT_SOURCE_DIR}" )
  endif()
  STRING( REGEX REPLACE "/" "_" plugname "${CURRENT_SUBDIR}" )
  set(plugin_name "${plugname}_${name}_${type}")
  set(codename "${name}_${type}.cc")
  if( NOT SP_ALLOW_UNDERSCORES )
    string(REGEX MATCH [_] has_underscore ${name})
    if( has_underscore )
      message(SEND_ERROR  "found underscore in plugin name: ${name}" )
    endif( has_underscore )
  endif()
  #message(STATUS "SIMPLE_PLUGIN: generating ${plugin_name}")
  add_library(${plugin_name} SHARED ${codename} )
  set(simple_plugin_liblist "${SP_DEFAULT_ARGS}")
  if( simple_plugin_liblist )
    target_link_libraries( ${plugin_name} ${simple_plugin_liblist} )
  endif( simple_plugin_liblist )
  if( NOT SP_NOINSTALL )
    install( TARGETS ${plugin_name}  DESTINATION ${flavorqual_dir}/lib )
  endif()
endmacro (simple_plugin name type)
