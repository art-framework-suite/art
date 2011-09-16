include(CetTest)

art_dictionary(NOINSTALL)

set(standard_libraries
  art_Persistency_Provenance
  art_Utilities
  art_Version
	${Boost_THREAD_LIBRARY}
  ${CINTEX}
  ${CORE}
  ${CINT}
	${REFLEX}
  -ldl
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

cet_test(RootClassMapping_t  NO_AUTO
  LIBRARIES
  test_Persistency_Provenance_dict
  ${CINTEX}
  ${TREE}
  ${NET}
  ${MATHCORE}
  ${RIO}
  ${THREAD}
  ${CORE}
  ${CINT}
	${REFLEX}
  -ldl
)
