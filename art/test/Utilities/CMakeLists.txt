# cet_test macro
set(default_art_test_libraries
    art::art_Utilities
    art::art_Version
    canvas::canvas)


add_subdirectory(tools)

cet_test(make_tool_t USE_BOOST_UNIT LIBRARIES PRIVATE ${default_art_test_libraries})

cet_test(ParameterSet_get_CLHEP_t
  LIBRARIES PRIVATE ${default_art_test_libraries} CLHEP::CLHEP
  )

cet_test(pointersEqual_t USE_BOOST_UNIT
  LIBRARIES PRIVATE ${default_art_test_libraries}
  )

cet_test(ScheduleID_t USE_BOOST_UNIT LIBRARIES PRIVATE art::art_Utilities)
cet_test(parent_path_t USE_BOOST_UNIT LIBRARIES PRIVATE art::art_Utilities)
