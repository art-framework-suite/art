# cet_test macro

art_dictionary(NO_DEFAULT_LIBRARIES NO_INSTALL NO_CHECK_CLASS_VERSION)

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

cet_test(uniform_type_name_test USE_BOOST_UNIT
  LIBRARIES
  art_Utilities
  )

cet_make_exec(TypeNameBranchName_t
  NO_INSTALL
  LIBRARIES
  art_Utilities
  cetlib
)

set(tnum 1)
foreach(types_file art_test_types.txt microboone_MC6.txt)
  if (tnum LESS 10)
    set(tnum_text "0${tnum}")
  else()
    set(tnum_text "${tnum}")
  endif()
  cet_test(TypeNameBranchName_test_${tnum_text} HANDBUILT
    TEST_EXEC TypeNameBranchName_t
    TEST_ARGS ${CMAKE_CURRENT_SOURCE_DIR}/TypeNameBranchName_t/${types_file}
    )
  math(EXPR tnum "${tnum} + 1")
endforeach()
