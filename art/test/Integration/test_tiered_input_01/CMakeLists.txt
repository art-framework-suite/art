set(levels events)
list(LENGTH levels len)

foreach(LEVEL RANGE ${${len}-1})

  list(GET levels LEVEL LEVELSTR)

  file(GLOB ifclfiles "${CMAKE_CURRENT_SOURCE_DIR}/fcl/*.fcl.in")
  foreach(ifclfile ${ifclfiles})
    string(REGEX REPLACE "\\.in$" "" fclfile ${ifclfile}) # Strip '.in'
    configure_file(${ifclfile} ${CMAKE_CURRENT_BINARY_DIR}/${fclfile} @ONLY)
  endforeach()

  cet_test(test_tiered_input_01a_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config make_m1.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/make_m1.fcl
    )

  cet_test(test_tiered_input_01b_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config read_m1.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/read_m1.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01a_${LEVELSTR}_t
    )

  cet_test(test_tiered_input_01c_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config make_m2.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/make_m2.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01a_${LEVELSTR}_t
    )

  cet_test(test_tiered_input_01d_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config read_m2.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/read_m2.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01c_${LEVELSTR}_t
    )

  cet_test(test_tiered_input_01e_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config make_m3.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/make_m3.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01c_${LEVELSTR}_t
    )

  cet_test(test_tiered_input_01f_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config read_both.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/read_both.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01e_${LEVELSTR}_t
    )

  cet_test(test_tiered_input_01g_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config copy_both.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/copy_both.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01e_${LEVELSTR}_t
    )

  cet_test(test_tiered_input_01h_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config copy_both_no_fastclone.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/copy_both_no_fastclone.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01e_${LEVELSTR}_t
    )

endforeach()