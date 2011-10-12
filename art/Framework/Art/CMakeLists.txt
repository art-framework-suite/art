foreach(cfg_source
    check_libs.cc
    )
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${cfg_source}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${cfg_source} @ONLY
    )
endforeach()

set( art_Framework_Art_sources
  InitRootHandlers.cc
  NovaConfigPostProcessor.cc
  artapp.cc
  find_config.cc
  novaapp.cc
  run_art.cc
  )

add_library(art_Framework_Art SHARED ${art_Framework_Art_sources} )
target_link_libraries(art_Framework_Art
  art_Framework_IO_Root
  art_Framework_Core
  art_Framework_Services_Registry
  art_Persistency_Common
  art_Persistency_Provenance
  art_Utilities
  )

install ( TARGETS
  art_Framework_Art
  DESTINATION
  ${flavorqual_dir}/lib
  )

set ( common_exec_link_libraries
  art_Framework_Core
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
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
  ${CPPUNIT}
  -ldl
)

set ( art_exec_link_libraries
  art_Persistency_Common_dict
  art_Framework_Art
  art_Framework_Services_Registry
  art_Utilities
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
)

set (ART_EXECS
  art
  gm2
  lar
  mu2e
  nova
)

foreach (art_exec ${ART_EXECS})
  add_executable( ${art_exec} ${art_exec}.cc )
  target_link_libraries( ${art_exec}
                         ${art_exec_link_libraries}
                         ${common_exec_link_libraries} )
  install ( TARGETS ${art_exec} DESTINATION ${flavorqual_dir}/bin )
endforeach (art_exec ${ART_EXECS})

# Versions of the ART execs supporting modules using BOOST unit test framework.
macro(art_ut_exec TARGET_STEM MAIN_FUNC)
  set(ART_UT_MAIN_FUNC ${MAIN_FUNC})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/art_ut.cc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_STEM}_ut.cc @ONLY
    )
  add_executable(${TARGET_STEM}_ut ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_STEM}_ut.cc)
  target_link_libraries(${TARGET_STEM}_ut
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${art_exec_link_libraries}
    ${common_exec_link_libraries} )
  install ( TARGETS ${TARGET_STEM}_ut DESTINATION ${flavorqual_dir}/bin )
  set_target_properties(${TARGET_STEM}_ut PROPERTIES
    COMPILE_DEFINITIONS "BOOST_TEST_DYN_LINK"
    )
endmacro()

art_ut_exec(art artapp)
art_ut_exec(nova novaapp)
art_ut_exec(mu2e novaapp)
art_ut_exec(gm2 novaapp)

add_executable( check_libs ${CMAKE_CURRENT_BINARY_DIR}/check_libs.cc )
target_link_libraries( check_libs ${common_exec_link_libraries} )
install ( TARGETS check_libs DESTINATION ${flavorqual_dir}/bin )

install_headers()
install_source()
