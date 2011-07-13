# cet_test macro
include(CetTest)

art_dictionary(NOINSTALL)

link_libraries(art_Utilities
  test_Utilities_dict
  ${MF_MESSAGELOGGER}
  ${CINT}
  ${REFLEX}
  )

cet_test(CPUTimer_t USE_BOOST_UNIT
  TEST_PROPERTIES ENVIRONMENT BOOST_TEST_SHOW_PROGRESS=yes
  )
cet_test(FriendlyName_t USE_BOOST_UNIT)
cet_test(TypeID_t USE_BOOST_UNIT)

foreach(test_cpp CRC32Calculator_t.cpp Digest_t.cpp MallocOpts_t.cpp)
  get_filename_component(tname ${test_cpp} NAME_WE )
  cet_test(${tname}
    SOURCES ${test_cpp}
    )
endforeach()

foreach(cppunit_test HRTime_t.cpp Math_t.cpp)
  get_filename_component(tname ${cppunit_test} NAME_WE )
  cet_test(${tname}
    SOURCES ${cppunit_test}
    LIBRARIES ${CPPUNIT}
    )
endforeach()
