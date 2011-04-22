include(CetTest)

link_libraries( art_Persistency_Provenance
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
    LIBRARIES ${CPPUNIT}
    )
endforeach()

foreach(test_source ModuleDescription_t.cpp ProcessConfiguration_t.cpp ProcessHistory_t.cpp)
  get_filename_component(test_name ${test_source} NAME_WE )
  cet_test(${test_name} SOURCES ${test_source})
endforeach()
