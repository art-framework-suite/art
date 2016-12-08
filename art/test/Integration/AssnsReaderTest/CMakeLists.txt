simple_plugin(AssnsReaderTest "module" NO_INSTALL USE_BOOST_UNIT)

foreach (tnum 1 2 3 4 5 6)
  set(read_dfiles
    fcl/AssnsReaderTest_${tnum}.fcl
    fcl/AssnsReaderTest_r.fcl
    fcl/AssnsReaderTest_r${tnum}.fcl
    )
  set(write_dfiles
    fcl/AssnsReaderTest_${tnum}.fcl
    fcl/AssnsReaderTest_w.fcl
    fcl/AssnsReaderTest_w${tnum}.fcl
    )
  if (tnum EQUAL 4 )
    list(APPEND write_dfiles fcl/AssnsReaderTest_r.fcl)
    list(APPEND read_dfiles fcl/AssnsReaderTest_w.fcl)
    set(test_properties_w TEST_PROPERTIES DEPENDS AssnsReaderTest_w1)
  else()
    set(test_properties_w)
  endif()
  cet_test(AssnsReaderTest_w${tnum} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS -- --rethrow-all -c "AssnsReaderTest_w${tnum}.fcl" -o out.root
    DATAFILES ${write_dfiles}
    ${test_properties_w}
    )
  cet_test(AssnsReaderTest_r${tnum} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS -- --rethrow-all -c "AssnsReaderTest_r${tnum}.fcl" ../AssnsReaderTest_w${tnum}.d/out.root
    REQUIRED_FILES ../AssnsReaderTest_w${tnum}.d/out.root
    DATAFILES ${read_dfiles}
    TEST_PROPERTIES DEPENDS AssnsReaderTest_w${tnum}
    )
endforeach()
