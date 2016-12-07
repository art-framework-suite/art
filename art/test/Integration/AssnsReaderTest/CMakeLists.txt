simple_plugin(AssnsReaderTest "module" NO_INSTALL USE_BOOST_UNIT)

foreach (tnum 1 2 3)
  cet_test(AssnsReaderTest_w${tnum} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS -- --rethrow-all -c "AssnsReaderTest_w${tnum}.fcl" -o out.root
    DATAFILES
    fcl/AssnsReaderTest_${tnum}.fcl
    fcl/AssnsReaderTest_w.fcl
    fcl/AssnsReaderTest_w${tnum}.fcl
    )
  cet_test(AssnsReaderTest_r${tnum} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS -- --rethrow-all -c "AssnsReaderTest_r${tnum}.fcl" ../AssnsReaderTest_w${tnum}.d/out.root
    REQUIRED_FILES ../AssnsReaderTest_w${tnum}.d/out.root
    DATAFILES
    fcl/AssnsReaderTest_${tnum}.fcl
    fcl/AssnsReaderTest_r.fcl
    fcl/AssnsReaderTest_r${tnum}.fcl
    TEST_PROPERTIES DEPENDS AssnsReaderTest_w${tnum}
    )
endforeach()
