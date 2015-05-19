# cet_test macro

art_dictionary(NO_INSTALL)

set(default_test_libraries
  art_Utilities
  test_Utilities_dict
  MF_MessageLogger
  ${ROOT_CINT}
  ${ROOT_REFLEX}
  )

cet_test(FriendlyName_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
  )

cet_test(InputTag_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
  )

cet_test(ParameterSet_get_CLHEP_t
  LIBRARIES ${default_test_libraries} ${CLHEP}
  )

cet_test(ParameterSet_get_artInputTag_t
  LIBRARIES ${default_test_libraries}
  TEST_ARGS "dummy"
  REF "${CMAKE_CURRENT_SOURCE_DIR}/ParameterSet_get_artInputTag_t-ref.txt"
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

foreach(test_cpp MallocOpts_t.cpp)
  get_filename_component(tname ${test_cpp} NAME_WE )
  cet_test(${tname}
    SOURCES ${test_cpp}
    LIBRARIES ${default_test_libraries}
    )
endforeach()

foreach(cppunit_test HRTime_t.cpp)
  get_filename_component(tname ${cppunit_test} NAME_WE )
  cet_test(${tname}
    SOURCES ${cppunit_test}
    LIBRARIES ${default_test_libraries} ${CPPUNIT}
    )
endforeach()

cet_test(ScheduleID_t USE_BOOST_UNIT)

cet_test(parent_path_t USE_BOOST_UNIT
  LIBRARIES art_Utilities)

cet_test(TypeNameBranchName_t
  LIBRARIES
  test_Utilities_dict
  art_Persistency_CetlibDictionaries_dict
  art_Persistency_StdDictionaries_dict
  art_Utilities
  cetlib
  ${ROOT_REFLEX}
  REF "${CMAKE_CURRENT_SOURCE_DIR}/TypeNameBranchName_t-ref.txt"
)

