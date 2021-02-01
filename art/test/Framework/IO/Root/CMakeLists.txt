cet_test(GetFileFormatVersion SOURCES test_GetFileFormatVersion.cpp
  LIBRARIES art_Framework_IO_RootVersion)

foreach (mode M S P)
  cet_test(config_dumper_${mode}_t HANDBUILT
    TEST_EXEC config_dumper
    TEST_ARGS -${mode} ../../../../Integration/Assns_w.d/out.root
    REF "${CMAKE_CURRENT_SOURCE_DIR}/config_dumper_${mode}_t-ref.txt"
    TEST_PROPERTIES
    DEPENDS Assns_w
    REQUIRED_FILES ../../../../Integration/Assns_w.d/out.root
    )
endforeach()

cet_test(RootOutputClosingCriteria_t
  USE_BOOST_UNIT
  LIBRARIES
  art_Framework_IO_Root
  )

cet_test(merge_process_histories_t
  USE_BOOST_UNIT
  LIBRARIES
  art_Framework_IO_Root
  )

cet_test(file_groups_t
  USE_BOOST_UNIT
  LIBRARIES
  art_Framework_IO_Root
  )

cet_test(EventMerger_common_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c event_merger_common_t.fcl
  DATAFILES fcl/event_merger_common_t.fcl fcl/outputs.fcl
)

cet_test(EventMerger_calo_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c event_merger_calo_t.fcl
  DATAFILES fcl/event_merger_calo_t.fcl fcl/outputs.fcl
  REQUIRED_FILES ../EventMerger_common_t.d/out.root
  TEST_PROPERTIES DEPENDS EventMerger_common_t
)

cet_test(EventMerger_tracker_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c event_merger_tracker_t.fcl
  DATAFILES fcl/event_merger_tracker_t.fcl fcl/outputs.fcl
  REQUIRED_FILES ../EventMerger_common_t.d/out.root
  TEST_PROPERTIES DEPENDS EventMerger_common_t
)

foreach(I RANGE 1 2)
  cet_test(EventMerger_combine_t${I} HANDBUILT
    TEST_EXEC art
    TEST_ARGS -c event_merger_combine_t${I}.fcl
    DATAFILES
      fcl/event_merger_combine_t${I}.fcl
      fcl/outputs.fcl
      fcl/analyze_combination.fcl
      files_to_combine_t.txt
    REQUIRED_FILES
    ../EventMerger_calo_t.d/out.root
    ../EventMerger_tracker_t.d/out.root
  )

  set_tests_properties(EventMerger_combine_t${I}
    PROPERTIES DEPENDS "EventMerger_calo_t;EventMerger_tracker_t"
  )

  cet_test(EventMerger_analyze_t${I} HANDBUILT
    TEST_EXEC art
    TEST_ARGS
      -c event_merger_analyze_t.fcl
      -s ../EventMerger_combine_t${I}.d/out.root
    DATAFILES
      fcl/event_merger_analyze_t.fcl
      fcl/analyze_combination.fcl
    REQUIRED_FILES ../EventMerger_combine_t${I}.d/out.root
    TEST_PROPERTIES DEPENDS EventMerger_combine_t${I}
  )
endforeach()
