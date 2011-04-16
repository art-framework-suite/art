# cet_test macro
include(CetTest)

link_libraries(art_Utilities
  ${MF_MESSAGELOGGER}
  ${CINT}
  ${REFLEX}
  )

foreach(test_cpp CRC32Calculator_t.cpp Digest_t.cpp MallocOpts_t.cpp)
  get_filename_component(tname ${test_cpp} NAME_WE )
  cet_test(${tname}
    SOURCES ${test_cpp}
    )
endforeach()

foreach(cppunit_test Utilities_t.cpp HRTime_t.cpp Math_t.cpp)
  get_filename_component(tname ${cppunit_test} NAME_WE )
  cet_test(${tname}
    SOURCES ${cppunit_test}
    LIBRARIES ${CPPUNIT}
    )
endforeach()
