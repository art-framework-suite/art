set( test_Framework_Art_libraries
  art_Framework_Services_Registry
  art_Utilities
  canvas_Utilities
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
  ${CMAKE_DL_LIBS}
 )


cet_test(artapp_t USE_BOOST_UNIT
  LIBRARIES
  art_Framework_Art
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  DATAFILES
  fcl/opt-empty.fcl
)

cet_test(artapp_basicSourceOptions_t
  USE_BOOST_UNIT
  LIBRARIES
  art_Framework_Art
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
    "\\*\\* ART_DEBUG_CONFIG is defined.*\\*\\* Config output to file"
    )
endmacro()

macro(config_test_debug_pass TNAME)
  SET_TESTS_PROPERTIES(${TNAME}
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Config output to file"
    )
endmacro()


####################################
# BasicOptionsHandler.
cet_test(BasicOptions_01_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c opt-empty.fcl
  --process-name TEST
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  PASS_REGULAR_EXPRESSION "ART_DEBUG_CONFIG is defined"
  PASS_REGULAR_EXPRESSION "Config output follows"
  DATAFILES fcl/opt-empty.fcl
  )

cet_test(BasicOptions_02_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -h
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "\nAllowed options:\n"
)

cet_test(BasicOptions_03_t
  LIBRARIES
  art_Framework_Art
  REF "${CMAKE_CURRENT_SOURCE_DIR}/BasicOptions_03-ref.txt"
  )

####################################
# BasicOutputOptionsHandler.

# Should pass and provide config output file for comparison in
# BasicOutputOptions_01_r against reference.
cet_test(BasicOutputOptions_01_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --TFileName hist.root
  -o out.root
  --tmpdir /tmp
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-empty.fcl
  )
config_test_pass(BasicOutputOptions_01_w)
config_ref_test(BasicOutputOptions_01)

# Detect ambiguity and exit with error.
cet_test(BasicOutputOptions_02_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --no-output -o f.root
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  PASS_REGULAR_EXPRESSION "\n  Output configuration is ambiguous: command-line specifies --output and --no-output simultaneously\\."
  DATAFILES fcl/opt-empty.fcl
)

# Should pass -- NOP
cet_test(BasicOutputOptions_03_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --no-output
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-empty.fcl
)
config_test_pass(BasicOutputOptions_03_w)
config_ref_test(BasicOutputOptions_03)

# Should pass -- NOP
cet_test(BasicOutputOptions_04_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o /dev/null
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-empty.fcl
)
config_test_pass(BasicOutputOptions_04_w)
config_ref_test(BasicOutputOptions_04)

# Should pass -- simple.
cet_test(BasicOutputOptions_05_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o /dev/null
  -c opt-simple-out.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-simple-out.fcl
)
config_test_pass(BasicOutputOptions_05_w)
config_ref_test(BasicOutputOptions_05)

# Should pass -- simple.
cet_test(BasicOutputOptions_06_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --no-output
  -c opt-simple-out.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-simple-out.fcl
)
config_test_pass(BasicOutputOptions_06_w)
config_ref_test(BasicOutputOptions_06)

# Should pass -- multi-path.
cet_test(BasicOutputOptions_07_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o /dev/null
  -c opt-multipath-out.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-multipath-out.fcl
)
config_test_pass(BasicOutputOptions_07_w)
config_ref_test(BasicOutputOptions_07)

# Should pass -- multi-path.
cet_test(BasicOutputOptions_08_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --no-output
  -c opt-multipath-out.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-multipath-out.fcl
)
config_test_pass(BasicOutputOptions_08_w)
config_ref_test(BasicOutputOptions_08)

# Should detect ambiguity -- multi-out.
cet_test(BasicOutputOptions_09_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o /dev/null
  -c opt-multiout.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  PASS_REGULAR_EXPRESSION "\n  Output configuration is ambiguous: configuration has multiple output modules\\. Cannot decide where to add specified output filename /dev/null\\."
  DATAFILES fcl/opt-multiout.fcl
)

# Should pass -- multi-out.
cet_test(BasicOutputOptions_10_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --no-output
  -c opt-multiout.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-multiout.fcl
)
config_test_pass(BasicOutputOptions_10_w)
config_ref_test(BasicOutputOptions_10)

# Should pass -- multi-out.
cet_test(BasicOutputOptions_11_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --no-output
  -c opt-no-end_paths-multiout.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-no-end_paths-multiout.fcl
)
config_test_pass(BasicOutputOptions_11_w)
config_ref_test(BasicOutputOptions_11)

# Specified stream name with empty outputs.
cet_test(BasicOutputOptions_12_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  -o myOut:out.root
  --tmpdir /tmp
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-empty.fcl
  )
config_test_pass(BasicOutputOptions_12_w)
config_ref_test(BasicOutputOptions_12)

# Should pass -- multi-out.
cet_test(BasicOutputOptions_13_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o o2:junk9.out
  -c opt-multiout.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-multiout.fcl
)
config_test_pass(BasicOutputOptions_13_w)
config_ref_test(BasicOutputOptions_13)

# Should fail -- command-line multi-out without stream spec.
cet_test(BasicOutputOptions_14_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o junk9.out -o junk8.out
  -c opt-empty.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "only the first specified output may omit the stream specification"
  DATAFILES fcl/opt-empty.fcl
)

# Should pass -- command-line and file multi-out with stream specs.
cet_test(BasicOutputOptions_15_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o o2:junk9.out -o o3:junk21.out
  -c opt-multiout.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-multiout.fcl
)
config_test_pass(BasicOutputOptions_15_w)
config_ref_test(BasicOutputOptions_15)

# Should fail -- command-line multi-out with late no-stream spec.
cet_test(BasicOutputOptions_16_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o o2:junk9.out -o junk8.out
  -c opt-empty.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "only the first specified output may omit the stream specification"
  DATAFILES fcl/opt-empty.fcl
)

####################################
# BasicSourceOptionsHandler.

# Should pass and provide config output file for comparison in
# BasicSourceOptions_01 against reference.
cet_test(BasicSourceOptions_01_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --estart 5 --nevts 4 --nskip 2
  -c opt-empty.fcl
  -s f0.txt
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-empty.fcl
  )

config_test_pass(BasicSourceOptions_01_w)

# Compare output from BasicSourceOptions_01_w against reference.
config_ref_test(BasicSourceOptions_01)

cet_test(BasicSourceOptions_02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c BasicSourceOptions_02.fcl
  -s f0.txt
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  PASS_REGULAR_EXPRESSION
  "Error: source files specified for EmptyEvent source\\."
  DATAFILES fcl/BasicSourceOptions_02.fcl
  )

cet_test(BasicSourceOptions_03 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --estart 5 --nevts 4 --nskip 2
  -c opt-empty.fcl
  -s f0.txt
  -S BasicSourceOptions-srclist.txt
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  PASS_REGULAR_EXPRESSION
  "--source-list \\(-S\\) and --source \\(-s\\) or non-option arguments are incompatible due to ordering ambiguities\\."
  DATAFILES fcl/opt-empty.fcl
  BasicSourceOptions-srclist.txt
  )

cet_test(BasicSourceOptions_04_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --estart 5 --nevts 4 --nskip 2
  -c BasicSourceOptions_04.fcl
  -S BasicSourceOptions-empty-srclist.txt
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/BasicSourceOptions_04.fcl
  BasicSourceOptions-empty-srclist.txt
  )

config_test_pass(BasicSourceOptions_04_w)

# Compare output from BasicSourceOptions_04_w against reference.
config_ref_test(BasicSourceOptions_04)

cet_test(BasicSourceOptions_05_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --estart 5 --nevts 4 --nskip 2
  -c BasicSourceOptions_04.fcl
  -S BasicSourceOptions-srclist.txt
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/BasicSourceOptions_04.fcl
  BasicSourceOptions-srclist.txt
  )

config_test_pass(BasicSourceOptions_05_w)

# Compare output from BasicSourceOptions_05_w against reference.
config_ref_test(BasicSourceOptions_05)

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
  --debug-config config.out
  -c opt-empty.fcl
  DATAFILES fcl/opt-empty.fcl
  )

config_test_debug_pass(DebugOptions_01_w)

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

# Save post-processed config and execute job as normal.
cet_test(DebugOptions_03_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --notrace --nomemcheck --rethrow-all --config-out=config.out
  -c DebugOptions_02_w.fcl
  DATAFILES fcl/DebugOptions_02_w.fcl
)

# Compare with reference.
config_ref_test(DebugOptions_03)

####################################
# FileCatalogOptionsHandler.

# Should fail due to requiring both main SAM options.
cet_test(FileCatalogOptions_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  PASS_REGULAR_EXPRESSION
  "  configurations services\\.CatalogInterface\\.webURI \\(--sam-web-uri\\) and
  services\\.FileCatalogMetadata\\.processID \\(--sam-process-id\\) must be specified
  together or not at all\\."
  DATAFILES fcl/opt-empty.fcl
)

# Should exit 1 after printing config -- no output means no metadata required.
cet_test(FileCatalogOptions_02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  PASS_REGULAR_EXPRESSION "ART_DEBUG_CONFIG is defined"
  PASS_REGULAR_EXPRESSION "Config output follows"
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
  "  Output \"out\" must be configured with dataTier \\(--sam-data-tier=out:<tier>\\) and streamName \\(--sam-stream-name=out:<stream>\\)\\.\n"
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

# Should pass and provide config output file for comparison in
# FileCatalogOptions_06_r.
cet_test(FileCatalogOptions_06_w HANDBUILT
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
  --sam-stream-name "wannasweetie"
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-empty.fcl
)

config_test_pass(FileCatalogOptions_06_w)

# Compare output from FileCatalogOptions_07_w against reference.
config_ref_test(FileCatalogOptions_06)

# Should pass and provide config output file for comparison in
# FileCatalogOptions_07_r.
cet_test(FileCatalogOptions_07_w HANDBUILT
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
  --sam-stream-name "out:wannabikkit"
  --sam-stream-name "wannasweetie"
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/opt-empty.fcl
)

config_test_pass(FileCatalogOptions_07_w)

# Compare output from FileCatalogOptions_07_w against reference.
config_ref_test(FileCatalogOptions_07)

# Should pass and provide config output file for comparison in
# FileCatalogOptions_08_r.
cet_test(FileCatalogOptions_08_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c simple-with-streamName.fcl
  --process-name=SAMTEST
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/simple-with-streamName.fcl
)

config_test_pass(FileCatalogOptions_08_w)

# Compare output from FileCatalogOptions_08_w against reference.
config_ref_test(FileCatalogOptions_08)

# Should pass and provide config output file for comparison in
# FileCatalogOptions_09_r.
cet_test(FileCatalogOptions_09_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c simple-with-streamName.fcl
  --process-name=SAMTEST
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  --sam-stream-name "wannasweetie"
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/simple-with-streamName.fcl
)

config_test_pass(FileCatalogOptions_09_w)

# Compare output from FileCatalogOptions_09_w against reference.
config_ref_test(FileCatalogOptions_09)

# Should pass and provide config output file for comparison in
# FileCatalogOptions_10_r.
cet_test(FileCatalogOptions_10_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c simple-with-streamName.fcl
  --process-name=SAMTEST
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  --sam-stream-name "out:wannabikkit"
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/simple-with-streamName.fcl
)

config_test_pass(FileCatalogOptions_10_w)

# Compare output from FileCatalogOptions_10_w against reference.
config_ref_test(FileCatalogOptions_10)

# Should pass and provide config output file for comparison in
# FileCatalogOptions_11_r.
cet_test(FileCatalogOptions_11_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c simple-with-dataTier.fcl
  --process-name=SAMTEST
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  --sam-data-tier "out:The one with all the flowers"
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/simple-with-dataTier.fcl
)

config_test_pass(FileCatalogOptions_11_w)

# Compare output from FileCatalogOptions_11_w against reference.
config_ref_test(FileCatalogOptions_11)

# Should pass and provide config output file for comparison in
# FileCatalogOptions_12_r.
cet_test(FileCatalogOptions_12_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c simple-with-dataTier.fcl
  --process-name=SAMTEST
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  --sam-data-tier "out:The one with all the flowers"
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/simple-with-dataTier.fcl
)

config_test_pass(FileCatalogOptions_12_w)

# Compare output from FileCatalogOptions_12_w against reference.
config_ref_test(FileCatalogOptions_12)

# Should pass and provide config output file for comparison in
# FileCatalogOptions_13_r.
cet_test(FileCatalogOptions_13_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c simple-with-dataTier.fcl
  --process-name=SAMTEST
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/simple-with-dataTier.fcl
)

config_test_pass(FileCatalogOptions_13_w)

# Compare output from FileCatalogOptions_13_w against reference.
config_ref_test(FileCatalogOptions_13)

# Should pass and provide config output file for comparison in
# FileCatalogOptions_14_r.
cet_test(FileCatalogOptions_14_w HANDBUILT
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

config_test_pass(FileCatalogOptions_14_w)

# Compare output from FileCatalogOptions_14_w against reference.
config_ref_test(FileCatalogOptions_14)

# Should all fail due to conflicting options
cet_test(FileCatalogOptions_15 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-inherit-file-type
  --sam-file-type "Chicken"
  -c opt-empty.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "  The options '--sam-inherit-file-type' and '--sam-file-type' are mutually exclusive\\."
  DATAFILES
  fcl/opt-empty.fcl
  )

cet_test(FileCatalogOptions_16 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-inherit-run-type
  --sam-run-type "Turkey"
  -c opt-empty.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "  The options '--sam-inherit-run-type' and '--sam-run-type' are mutually exclusive\\."
  DATAFILES
  fcl/opt-empty.fcl
  )

cet_test(FileCatalogOptions_17 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-inherit-metadata
  --sam-file-type "Duck"
  -c opt-empty.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "  The options '--sam-inherit-metadata' and '--sam-file-type' are mutually exclusive\\."
  DATAFILES
  fcl/opt-empty.fcl
)

cet_test(FileCatalogOptions_18 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-inherit-metadata
  --sam-run-type "Turducken"
  -c opt-empty.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "  The options '--sam-inherit-metadata' and '--sam-run-type' are mutually exclusive\\."
  DATAFILES
  fcl/opt-empty.fcl
  )

####################################
# Handling deprecated configurations
#  Tests 3-5 trigger exceptions that do not result in config.out
#  statements--i.e., they cannot be used in comparing configurations
#  since the exception is thrown before the configuration is written
#  out.
foreach(i RANGE 1 5)
  cet_test(DeprecatedFileModeParameter_t_0${i} HANDBUILT
    TEST_EXEC art
    TEST_ARGS -c deprecated_fileMode_0${i}.fcl --debug-config=config.out
    DATAFILES fcl/deprecated_fileMode_0${i}.fcl
    TEST_PROPERTIES WILL_FAIL TRUE
    )
endforeach()

foreach(i RANGE 1 2)
  cet_test(DeprecatedFileModeParameter_r_0${i} HANDBUILT
    TEST_EXEC diff
    TEST_ARGS ../DeprecatedFileModeParameter_t_0${i}.d/config.out DeprecatedFileMode_0${i}-ref.txt
    DATAFILES DeprecatedFileMode_0${i}-ref.txt
    TEST_PROPERTIES DEPENDS DeprecatedFileModeParameter_t_0${i}
    )
endforeach()
