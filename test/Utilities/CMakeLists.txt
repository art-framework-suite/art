# cet_test macro

art_dictionary(NOINSTALL)

add_library(test_Utilities SHARED openmpTestFunc.cc)
target_link_libraries(test_Utilities ${Boost_THREAD_LIBRARY})

set(openmp_test_libraries
  art_Framework_Core
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
  -ldl
)

set(default_test_libraries
  art_Utilities
  test_Utilities_dict
  ${MF_MESSAGELOGGER}
  ${ROOT_CINT}
  ${ROOT_REFLEX}
  )

cet_test(openmp_simple_t LIBRARIES test_Utilities)
cet_test(openmp_dlopen_t NO_AUTO
  LIBRARIES
  ${openmp_test_libraries}
)

cet_test(openmp_dlopen_standalone_t HANDBUILT
  TEST_EXEC openmp_dlopen_t
  TEST_ARGS "libtest_Utilities.so" "openmpTestFunc"
  )

cet_test(openmp_dlopen_module_t HANDBUILT
  TEST_EXEC openmp_dlopen_t
  TEST_ARGS "libtest_Integration_OpenMPSimpleTest_module.so" "openmpTestFunc"
  )

cet_test(openmp_LibraryManager_t
  TEST_ARGS "test/Integration/OpenMPSimpleTest" "openmpTestFunc"
  LIBRARIES
  ${openmp_test_libraries}
  )

cet_test(CPUTimer_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
  TEST_PROPERTIES ENVIRONMENT BOOST_TEST_SHOW_PROGRESS=yes
  )

cet_test(FriendlyName_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
  )

cet_test(TypeID_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
  )

cet_test(pointersEqual_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
  )

cet_test(ensurePointer_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
  )

foreach(test_cpp CRC32Calculator_t.cpp Digest_t.cpp MallocOpts_t.cpp)
  get_filename_component(tname ${test_cpp} NAME_WE )
  cet_test(${tname}
    SOURCES ${test_cpp}
    LIBRARIES ${default_test_libraries}
    )
endforeach()

foreach(cppunit_test HRTime_t.cpp Math_t.cpp)
  get_filename_component(tname ${cppunit_test} NAME_WE )
  cet_test(${tname}
    SOURCES ${cppunit_test}
    LIBRARIES ${default_test_libraries} ${CPPUNIT}
    )
endforeach()

cet_test(LibraryManager_t USE_BOOST_UNIT NO_AUTO
  LIBRARIES 
  art_Utilities
  ${Boost_FILESYSTEM_LIBRARY}
  ${ROOT_CINTEX}
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
  -ldl
)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LibraryManager_t.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/LibraryManager_t.sh
  @ONLY
  )
cet_test(LibraryManager_t.sh HANDBUILT
  TEST_EXEC ${CMAKE_CURRENT_BINARY_DIR}/LibraryManager_t.sh
  )

cet_test(ScheduleID_t USE_BOOST_UNIT)
