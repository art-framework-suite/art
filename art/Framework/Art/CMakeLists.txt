# Configure file to handle differences for Mac.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/check_libs.cc.in
  ${CMAKE_CURRENT_BINARY_DIR}/check_libs.cc @ONLY
  )

####################################
# Configure for desired default exception handling.
SET(ART_MAIN_FUNC artapp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/artapp.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/${ART_MAIN_FUNC}.h @ONLY
  )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/artapp.cc.in
  ${CMAKE_CURRENT_BINARY_DIR}/${ART_MAIN_FUNC}.cc @ONLY
  )

SET(ART_MAIN_FUNC mu2eapp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/artapp.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/${ART_MAIN_FUNC}.h @ONLY
  )

SET(ART_RETHROW_DEFAULT TRUE)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/artapp.cc.in
  ${CMAKE_CURRENT_BINARY_DIR}/${ART_MAIN_FUNC}.cc @ONLY
  )
####################################

add_subdirectory(detail)
add_subdirectory(detail/md-collector)
add_subdirectory(detail/md-summary)

set(art_Framework_Art_sources
  BasicOptionsHandler.cc
  BasicPostProcessor.cc
  BasicSourceOptionsHandler.cc
  BasicOutputOptionsHandler.cc
  DebugOptionsHandler.cc
  FileCatalogOptionsHandler.cc
  InitRootHandlers.cc
  OptionsHandler.cc
  ${CMAKE_CURRENT_BINARY_DIR}/artapp.cc
  ${CMAKE_CURRENT_BINARY_DIR}/mu2eapp.cc
  find_config.cc
  run_art.cc
  detail/MetadataRegexHelpers.cc
  detail/PrintPluginMetadata.cc
  detail/ServiceNames.cc
  detail/fillSourceList.cc
  detail/get_LibraryInfoCollection.cc
  detail/get_MetadataCollector.cc
  detail/get_MetadataSummary.cc
  detail/handle_deprecated_configs.cc
  )

art_make_library( LIBRARY_NAME art_Framework_Art
                  SOURCE ${art_Framework_Art_sources}
                  LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY}
  art_Framework_IO_Root
  art_Framework_EventProcessor
  art_Framework_Core
  art_Framework_Services_Registry
  art_Persistency_Common
  art_Persistency_Provenance canvas_Persistency_Provenance
  art_Utilities canvas_Utilities
  ${ROOT_HIST}
  ${ROOT_MATRIX}
  )

set (ART_EXEC_ROOT_LIBS
  ${ROOT_CINTEX}
  ${ROOT_PHYSICS}
  ${ROOTSYS}/lib/libGraf.so
  ${ROOT_TREE}
  ${ROOT_HIST}
  ${ROOT_MATRIX}
  ${ROOT_NET}
  ${ROOT_MATHCORE}
  ${ROOT_RIO}
  ${ROOT_THREAD}
  ${ROOT_CORE}
  ${ROOT_CINT}
  ${ROOT_REFLEX}
)

# Build an art exec.
macro(art_exec TARGET_STEM IN_STEM MAIN_FUNC)
  cet_parse_args(AE "LIBRARIES" "" ${ARGN})
  set(ART_MAIN_FUNC ${MAIN_FUNC})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${IN_STEM}.cc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_STEM}.cc @ONLY
    )
  art_make_exec(NAME ${TARGET_STEM}
    SOURCE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_STEM}.cc
    ${AE_DEFAULT_ARGS}
    LIBRARIES
    ${AE_LIBRARIES}
    art_Framework_Art
    # ROOT libraries added for use convenience ONLY -- they will go away
    # eventually.
    ${ART_EXEC_ROOT_LIBS}
    MF_MessageLogger
    )
endmacro()

# Standard execs
art_exec(art art artapp)
art_exec(gm2 art artapp)
art_exec(lar art artapp)
art_exec(mu2e art mu2eapp)
art_exec(nova art artapp)

# Execs with Boost unit testing enabled for modules.
art_exec(art_ut art_ut artapp LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
art_exec(gm2_ut art_ut artapp LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
art_exec(lar_ut art_ut artapp LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
art_exec(mu2e_ut art_ut mu2eapp LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
art_exec(nova_ut art_ut artapp LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

art_make_exec(NAME check_libs
  SOURCE ${CMAKE_CURRENT_BINARY_DIR}/check_libs.cc
  LIBRARIES
  art_Utilities canvas_Utilities
  cetlib
  )

install_headers()
install_source()
