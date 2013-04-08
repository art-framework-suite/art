# macros for building plugin libraries
#
# The plugin type is expected to be service, source, or module,
# but we do not enforce this.
#
# USAGE:
# simple_plugin( <name> <plugin type>
#                [library list]
#                [ALLOW_UNDERSCORES]
#                [BASENAME_ONLY]
#                [NO_INSTALL]
#   )
#
# The plugin library's name is constructed from the specified name, its
# specified plugin type (eg service, module, source), and (unless
# BASENAME_ONLY is specified) the package subdirectory path (replacing
# "/" with "_").
#
# Options:
#
# ALLOW_UNDERSCORES
#
#   Allow underscores in subdirectory names. Discouraged, as it creates
#   a possible ambiguity in the encoded plugin library name (art_test/XX
#   is indistinguishable from art/test/XX).
#
# BASENAME_ONLY
#
#    Omit the subdirectory path from the library name. Discouraged, as
#    it creates an ambiguity between modules with the same source
#    filename in different packages or different subdirectories within
#    the same package. The latter case is not possible however, because
#    CMake will throw an error because the two CMake targets will have
#    the same name and that is not permitted.
#
# NO_INSTALL
#
#    If specified, the plugin library will not be part of the installed
#    product (use for test modules, etc.).
#
########################################################################

# simple plugin libraries
include(CetParseArgs)
macro (simple_plugin name type)
  cet_parse_args(SP "" "USE_BOOST_UNIT;ALLOW_UNDERSCORES;BASENAME_ONLY;NO_INSTALL;NOINSTALL" ${ARGN})
  if (NOINSTALL)
    message(SEND_ERROR "simple_plugin now requires NO_INSTALL instead of NOINSTALL")
  endif()
  if (SP_BASENAME_ONLY)
    set(plugin_name "${name}_${type}")
  else()
    #message( STATUS "simple_plugin: PACKAGE_TOP_DIRECTORY is ${PACKAGE_TOP_DIRECTORY}")
    # base name on current subdirectory
    if( PACKAGE_TOP_DIRECTORY )
      STRING( REGEX REPLACE "^${PACKAGE_TOP_DIRECTORY}/(.*)" "\\1" CURRENT_SUBDIR "${CMAKE_CURRENT_SOURCE_DIR}" )
    else()
      STRING( REGEX REPLACE "^${CMAKE_SOURCE_DIR}/(.*)" "\\1" CURRENT_SUBDIR "${CMAKE_CURRENT_SOURCE_DIR}" )
    endif()
    if(NOT SP_ALLOW_UNDERSCORES )
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
  endif()
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
  if(ART_FRAMEWORK_CORE)
    # using art as a product
    if("${type}" STREQUAL "service")
      list(INSERT simple_plugin_liblist 0 ${ART_FRAMEWORK_SERVICES_REGISTRY} ${SIGC})
    elseif("${type}" STREQUAL "module" OR "${type}" STREQUAL "source")
      list(INSERT simple_plugin_liblist 0
	      ${ART_FRAMEWORK_CORE}
	      ${ART_FRAMEWORK_PRINCIPAL}
	      ${ART_PERSISTENCY_COMMON}
	      ${ART_PERSISTENCY_PROVENANCE}
	      ${ART_UTILITIES}
        ${CETLIB}
	      ${ROOT_CORE}
	      )
    endif()
  else()
    # this block is used when building art
    if("${type}" STREQUAL "service")
      list(INSERT simple_plugin_liblist 0 art_Framework_Services_Registry)
    elseif("${type}" STREQUAL "module" OR "${type}" STREQUAL "source")
      list(INSERT simple_plugin_liblist 0
        art_Framework_Core
        art_Framework_Principal
        art_Persistency_Provenance
        art_Utilities
        ${ROOT_CORE}
        )
    endif()
  endif()
  if( simple_plugin_liblist )
    target_link_libraries( ${plugin_name} ${simple_plugin_liblist} )
  endif( simple_plugin_liblist )
  if( NOT SP_NO_INSTALL )
    install( TARGETS ${plugin_name}  DESTINATION ${flavorqual_dir}/lib )
    cet_add_to_library_list( ${plugin_name} )
  endif()
endmacro (simple_plugin name type)
