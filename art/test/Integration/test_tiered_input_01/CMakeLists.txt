set(levels events subruns runs)
list(LENGTH levels len)
math(EXPR limit "${len}-1")

file(GLOB ifclfiles "${CMAKE_CURRENT_SOURCE_DIR}/fcl/*.fcl.in")

foreach(LEVEL RANGE ${limit})

  # LEVELSTR is used to during the configure_file step
  list(GET levels ${LEVEL} LEVELSTR)

  foreach(ifclfile ${ifclfiles})
    # Strip '.in' and insert the appropriate level string
    string(REGEX REPLACE ".*/([^/]+)\\.fcl\\.in$" "\\1_${LEVELSTR}.fcl" fclfile ${ifclfile})
    configure_file(${ifclfile} ${CMAKE_CURRENT_BINARY_DIR}/${fclfile} @ONLY)
  endforeach()

  cet_test(test_tiered_input_01a_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config make_m1_${LEVELSTR}.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/make_m1_${LEVELSTR}.fcl
    )

  cet_test(test_tiered_input_01b_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config read_m1_${LEVELSTR}.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/read_m1_${LEVELSTR}.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01a_${LEVELSTR}_t
    )

  cet_test(test_tiered_input_01c_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config make_m2_${LEVELSTR}.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/make_m2_${LEVELSTR}.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01a_${LEVELSTR}_t
    )

  cet_test(test_tiered_input_01d_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config read_m2_${LEVELSTR}.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/read_m2_${LEVELSTR}.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01c_${LEVELSTR}_t
    )

  cet_test(test_tiered_input_01e_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config make_m3_${LEVELSTR}.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/make_m3_${LEVELSTR}.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01c_${LEVELSTR}_t
    )

  cet_test(test_tiered_input_01f_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config read_both_${LEVELSTR}.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/read_both_${LEVELSTR}.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01e_${LEVELSTR}_t
    )

  cet_test(test_tiered_input_01g_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config copy_both_${LEVELSTR}.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/copy_both_${LEVELSTR}.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01e_${LEVELSTR}_t
    )

  cet_test(test_tiered_input_01h_${LEVELSTR}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --config copy_both_no_fastclone_${LEVELSTR}.fcl --rethrow-all
    DATAFILES
    ${CMAKE_CURRENT_BINARY_DIR}/copy_both_no_fastclone_${LEVELSTR}.fcl
    TEST_PROPERTIES DEPENDS test_tiered_input_01e_${LEVELSTR}_t
    )

endforeach()