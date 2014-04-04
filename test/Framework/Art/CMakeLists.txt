link_libraries(
  art_Framework_Services_Registry
  art_Utilities
  art_Framework_Core
  ${Boost_FILESYSTEM_LIBRARY}
  ${ROOT_CINTEX}
  ${ROOT_TREE}
  ${ROOT_HIST}
  ${ROOT_MATRIX}
  ${ROOT_NET}
  ${ROOT_MATHCORE}
  ${ROOT_THREAD}
  ${ROOT_RIO}
	${ROOT_CORE}
  ${ROOT_CINT}
	${ROOT_REFLEX}
  ${CPPUNIT}
  -ldl
 )

cet_test(artapp_t USE_BOOST_UNIT
  LIBRARIES
  art_Framework_Art
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  DATAFILES
  fcl/empty_config.fcl
)

########################################################################
# OptionsHandler tests.

####################################
# config_ref_test
#
# Make reference comparisons easier.

macro(config_ref_test TNAME)
  cet_test(${TNAME}_r HANDBUILT
    TEST_EXEC diff
    TEST_ARGS -u ${TNAME}-ref.txt
    ../${TNAME}_w.d/config.out
    DATAFILES ${TNAME}-ref.txt
    TEST_PROPERTIES DEPENDS ${TNAME}_w
    )
endmacro()

####################################
# config_file_pass
#
# Ensure the config file has been written out to pass the test.

macro(config_test_pass TNAME)
  SET_TESTS_PROPERTIES(${TNAME}
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ART_DEBUG_CONFIG is defined: config debug output to file"
    )
endmacro()


####################################
# BasicOptionsHandler.
cet_test(BasicOptions_01_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c opt-empty.fcl
  --process-name TEST
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  PASS_REGULAR_EXPRESSION
  "ART_DEBUG_CONFIG is defined: config debug output follows"
  DATAFILES fcl/opt-empty.fcl
  )

cet_test(BasicOptions_02_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -h
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "\nAllowed options:\n"
)

####################################
# BasicOutputOptionsHandler.

# Should pass and provide config output file for comparison in
# BasicOutputOptions_01_r against reference.
cet_test(BasicOutputOptions_01_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --TFileName hist.root
  -o out.root
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-empty.fcl
  )

config_test_pass(BasicOutputOptions_01_w)

# Compare output from BasicOutputOptions_01_w against reference.
config_ref_test(BasicOutputOptions_01)


####################################
# BasicSourceOptionsHandler.

# Should pass and provide config output file for comparison in
# BasicSourceOptions_01 against reference.
cet_test(BasicSourceOptions_01_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --estart 5 --nevts 4 --nskip 2
  -c empty_config.fcl
  -s f0.txt
  -S BasicSourceOptions-srclist.txt
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/empty_config.fcl
  BasicSourceOptions-srclist.txt
  )

config_test_pass(BasicSourceOptions_01_w)

# Compare output from BasicSourceOptions_01_w against reference.
config_ref_test(BasicSourceOptions_01)

cet_test(BasicSourceOptions_02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c BasicSourceOptions_02.fcl
  -s f0.txt
  -S BasicSourceOptions-srclist.txt
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  PASS_REGULAR_EXPRESSION
  "Error: source files specified for EmptyEvent source\\."
  DATAFILES fcl/BasicSourceOptions_02.fcl
  BasicSourceOptions-srclist.txt
  )

####################################
# BasicPostProcessor.

# Should pass and provide config output file for comparison in
# BasicPostProcessor_01_r against reference.
cet_test(BasicPostProcessor_01_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c BasicPostProcessor_01_w.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/BasicPostProcessor_01_w.fcl
  )

config_test_pass(BasicPostProcessor_01_w)

# Compare output from BasicPostProcessor_01_w against reference.
config_ref_test(BasicPostProcessor_01)

####################################
# DebugOptionsHandler.

# Should pass and provide config output file for comparison in
# DebugOptions_01_r against reference.
cet_test(DebugOptions_01_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --trace --memcheck --rethrow-default
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-empty.fcl
  )

config_test_pass(DebugOptions_01_w)

# Compare output from DebugOptions_01_w against reference.
config_ref_test(DebugOptions_01)

# Should pass and provide config output file for comparison in
# DebugOptions_02_r against reference.
cet_test(DebugOptions_02_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --notrace --nomemcheck --rethrow-all
  -c DebugOptions_02_w.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/DebugOptions_02_w.fcl
  )

config_test_pass(DebugOptions_02_w)

# Compare output from DebugOptions_02_w against reference.
config_ref_test(DebugOptions_02)


####################################
# FileCatalogOptionsHandler.

# Should fail due to requiring both main SAM options.
cet_test(FileCatalogOptions_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  PASS_REGULAR_EXPRESSION
  "--sam-web-uri and --sam-process-id must be used together or not at all\\."
  DATAFILES fcl/opt-empty.fcl
)

# Should exit 1 after printing config -- no output means no metadata required.
cet_test(FileCatalogOptions_02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  PASS_REGULAR_EXPRESSION
  "ART_DEBUG_CONFIG is defined: config debug output follows"
  DATAFILES fcl/opt-empty.fcl
)

# Should fail due to missing metadata.
cet_test(FileCatalogOptions_03 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  PASS_REGULAR_EXPRESSION
  "SAM metadata information is required -- missing metadata:\n  --sam-application-family\n  --sam-application-version\n  --sam-file-type\n  --sam-data-tier\n"
  DATAFILES fcl/opt-empty.fcl
)

# Should fail due to missing / default process name.
cet_test(FileCatalogOptions_04 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c opt-empty.fcl
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  PASS_REGULAR_EXPRESSION
  "Non-empty / default process_name required for SAM metadata\\."
  DATAFILES fcl/opt-empty.fcl
)

# Should pass and provide config output file for comparison in
# FileCatalogOptions_05_r.
cet_test(FileCatalogOptions_05_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c opt-empty.fcl
  --process-name=SAMTEST
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-empty.fcl
)

config_test_pass(FileCatalogOptions_05_w)

# Compare output from FileCatalogOptions_05_w against reference.
config_ref_test(FileCatalogOptions_05)
