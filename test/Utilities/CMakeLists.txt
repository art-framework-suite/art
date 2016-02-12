# cet_test macro
set(default_art_test_libraries
  art_Utilities
  canvas_Utilities
  cetlib
  MF_MessageLogger
  ${ROOT_CINT}
  ${ROOT_REFLEX}
  )

cet_test(InputTag_t USE_BOOST_UNIT
  LIBRARIES ${default_art_test_libraries}
  )

cet_test(ParameterSet_get_CLHEP_t
  LIBRARIES ${default_art_test_libraries} ${CLHEP}
  )

cet_test(ParameterSet_get_artInputTag_t
  LIBRARIES ${default_art_test_libraries}
  TEST_ARGS "dummy"
  REF "${CMAKE_CURRENT_SOURCE_DIR}/ParameterSet_get_artInputTag_t-ref.txt"
  )

cet_test(pointersEqual_t USE_BOOST_UNIT
  LIBRARIES ${default_art_test_libraries}
  )

foreach(test_cpp MallocOpts_t.cpp)
  get_filename_component(tname ${test_cpp} NAME_WE )
  cet_test(${tname}
    SOURCES ${test_cpp}
    LIBRARIES ${default_art_test_libraries}
    )
endforeach()

foreach(cppunit_test HRTime_t.cpp)
  get_filename_component(tname ${cppunit_test} NAME_WE )
  cet_test(${tname}
    SOURCES ${cppunit_test}
    LIBRARIES ${default_art_test_libraries} ${CPPUNIT}
    )
endforeach()

cet_test(ScheduleID_t USE_BOOST_UNIT)

cet_test(parent_path_t USE_BOOST_UNIT
  LIBRARIES art_Utilities canvas_Utilities)
