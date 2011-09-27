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
cet_test(ensurePointer_t USE_BOOST_UNIT)

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

cet_test(LibraryManager_t USE_BOOST_UNIT NO_AUTO)
# Long link list required because the test will attempt to load
# dictionaries. One might argue that this qualifies it as an integration
# test.
target_link_libraries(LibraryManager_t
  art_Framework_Core
  art_Framework_Services_Registry
  art_Framework_Principal
  art_Utilities
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
  ${CINTEX}
  ${TREE}
  ${HIST}
  ${MATRIX}
  ${NET}
  ${MATHCORE}
  ${THREAD}
  ${RIO}
	${CORE}
  ${CINT}
	${REFLEX}
  ${CPPUNIT}
  -ldl
  )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LibraryManager_t.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/LibraryManager_t.sh
  @ONLY
  )
cet_test(LibraryManager_t.sh HANDBUILT
  TEST_EXEC ${CMAKE_CURRENT_BINARY_DIR}/LibraryManager_t.sh
  )
