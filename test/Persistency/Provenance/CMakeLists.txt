
art_dictionary(NO_INSTALL)

set(standard_libraries
  art_Persistency_Provenance
  art_Utilities
  art_Version
	${Boost_THREAD_LIBRARY}
  ${ROOT_CINTEX}
  ${ROOT_CORE}
  ${ROOT_CINT}
	${ROOT_REFLEX}
  -ldl
  )

cet_test(EventID_t USE_BOOST_UNIT
  LIBRARIES art_Persistency_Provenance
  )

file(GLOB cppunit_files *.cppunit.cc)
foreach(cppunit_source ${cppunit_files})
  get_filename_component(test_name ${cppunit_source} NAME_WE )
  cet_test(${test_name} SOURCES testRunner.cpp ${cppunit_source}
    LIBRARIES ${CPPUNIT} ${standard_libraries}
    )
endforeach()

foreach(test_source ModuleDescription_t.cpp ProcessConfiguration_t.cpp ProcessHistory_t.cpp)
  get_filename_component(test_name ${test_source} NAME_WE )
  cet_test(${test_name} SOURCES ${test_source} LIBRARIES ${standard_libraries})
endforeach()

cet_test(RootClassMapping_t USE_BOOST_UNIT
  LIBRARIES
  test_Persistency_Provenance_dict
  ${ROOT_CINTEX}
  ${ROOT_TREE}
  ${ROOT_NET}
  ${ROOT_MATHCORE}
  ${ROOT_RIO}
  ${ROOT_THREAD}
  ${ROOT_CORE}
  ${ROOT_CINT}
	${ROOT_REFLEX}
  -ldl
  # When ROOT is fixed WILL_FAIL should be removed and the code in ASSNS
  # (and associated ioread rules) should be fixed accordingly.
  TEST_PROPERTIES WILL_FAIL true
)
