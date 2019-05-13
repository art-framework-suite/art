cet_test(artapp_t USE_BOOST_UNIT LIBRARIES art_Framework_Art)
cet_test(artapp_basicSourceOptions_t USE_BOOST_UNIT LIBRARIES art_Framework_Art)
cet_test(event_start_t USE_BOOST_UNIT LIBRARIES art_Framework_Art)
cet_test(fhicl_key_t USE_BOOST_UNIT LIBRARIES art_Framework_Art)
cet_test(prune_config_t USE_BOOST_UNIT LIBRARIES art_Framework_Art)

########################################################################
# OptionsHandler tests.

####################################
# config_file_pass
#
# Ensure the config file has been written out to pass the test.

macro(config_test_pass TNAME)
  SET_TESTS_PROPERTIES(${TNAME}
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "\\*\\* ART_DEBUG_CONFIG is defined.*\\*\\* Config output to file")
endmacro()

macro(config_test_debug_pass TNAME)
  SET_TESTS_PROPERTIES(${TNAME}
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Config output to file")
endmacro()


####################################
# BasicOptionsHandler.
cet_test(BasicOptions_01_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c /dev/null
  --process-name TEST
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  PASS_REGULAR_EXPRESSION "ART_DEBUG_CONFIG is defined"
  PASS_REGULAR_EXPRESSION "Config output follows")

cet_test(BasicOptions_02_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -h
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "Basic options")

cet_test(BasicOptions_03_t
  LIBRARIES art_Framework_Art
  REF "${CMAKE_CURRENT_SOURCE_DIR}/BasicOptions_03-ref.txt")

add_subdirectory(PrintAvailable)

####################################
# BasicOutputOptionsHandler.
# FIXME: Tests 1 and 12 have moved to test/Configuration.

# Detect ambiguity and exit with error.
cet_test(BasicOutputOptions_02_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --no-output -o f.root
  -c /dev/null
  --debug-config=config.out
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "\n  Output configuration is ambiguous: command-line specifies --output and --no-output simultaneously\\.")

# Should pass -- NOP
cet_test(BasicOutputOptions_03_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --no-output
  -c /dev/null
  --debug-config=config.out)
config_test_debug_pass(BasicOutputOptions_03_w)
config_ref_test(BasicOutputOptions_03)

# Should pass -- NOP
cet_test(BasicOutputOptions_04_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o /dev/null
  -c /dev/null
  --debug-config=config.out)
config_test_debug_pass(BasicOutputOptions_04_w)
config_ref_test(BasicOutputOptions_04
  OUTPUT_FILTERS DEFAULT ${filterOutputModule})

# Should pass -- simple.
cet_test(BasicOutputOptions_05_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o /dev/null
  -c simple-out.fcl
  --debug-config=config.out
  DATAFILES fcl/simple-out.fcl)
config_test_debug_pass(BasicOutputOptions_05_w)
config_ref_test(BasicOutputOptions_05)

# Should pass -- simple.
cet_test(BasicOutputOptions_06_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --no-output
  -c simple-out.fcl
  --debug-config=config.out
  DATAFILES fcl/simple-out.fcl)
config_test_debug_pass(BasicOutputOptions_06_w)
config_ref_test(BasicOutputOptions_06)

# Should pass -- multi-path.
cet_test(BasicOutputOptions_07_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o /dev/null
  -c multipath-out.fcl
  --debug-config=config.out
  DATAFILES fcl/multipath-out.fcl)
config_test_debug_pass(BasicOutputOptions_07_w)
config_ref_test(BasicOutputOptions_07)

# Should pass -- multi-path.
cet_test(BasicOutputOptions_08_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --no-output
  -c multipath-out.fcl
  --debug-config=config.out
  DATAFILES fcl/multipath-out.fcl)
config_test_debug_pass(BasicOutputOptions_08_w)
config_ref_test(BasicOutputOptions_08)

# Should detect ambiguity -- multi-out.
cet_test(BasicOutputOptions_09_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o /dev/null
  -c multiout.fcl
  --debug-config=config.out
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "\n  Output configuration is ambiguous: configuration has multiple output modules\\. Cannot decide where to add specified output filename /dev/null\\."
  DATAFILES fcl/multiout.fcl)

# Should pass -- multi-out.
cet_test(BasicOutputOptions_10_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --no-output
  -c multiout.fcl
  --debug-config=config.out
  DATAFILES fcl/multiout.fcl)
config_test_debug_pass(BasicOutputOptions_10_w)
config_ref_test(BasicOutputOptions_10)

# Should pass -- multi-out.
cet_test(BasicOutputOptions_11_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --no-output
  -c no-end_paths-multiout.fcl
  --debug-config=config.out
  DATAFILES fcl/no-end_paths-multiout.fcl)
config_test_debug_pass(BasicOutputOptions_11_w)
config_ref_test(BasicOutputOptions_11)

# Should pass -- multi-out.
cet_test(BasicOutputOptions_13_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o o2:junk9.out
  -c multiout.fcl
  --debug-config=config.out
  DATAFILES fcl/multiout.fcl)
config_test_debug_pass(BasicOutputOptions_13_w)
config_ref_test(BasicOutputOptions_13)

# Should fail -- command-line multi-out without stream spec.
cet_test(BasicOutputOptions_14_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o junk9.out -o junk8.out
  -c /dev/null
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "only the first specified output may omit the stream specification")

# Should pass -- command-line and file multi-out with stream specs.
cet_test(BasicOutputOptions_15_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o o2:junk9.out -o o3:junk21.out
  -c multiout.fcl
  --debug-config=config.out
  DATAFILES fcl/multiout.fcl)
config_test_debug_pass(BasicOutputOptions_15_w)
config_ref_test(BasicOutputOptions_15
  OUTPUT_FILTERS DEFAULT ${filterOutputModule})

# Should fail -- command-line multi-out with late no-stream spec.
cet_test(BasicOutputOptions_16_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -o o2:junk9.out -o junk8.out
  -c /dev/null
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "only the first specified output may omit the stream specification")

####################################
# BasicSourceOptionsHandler.

# Should pass and provide config output file for comparison in
# BasicSourceOptions_01 against reference.
cet_test(BasicSourceOptions_01_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --estart 5 --nevts 4 --nskip 2
  -c /dev/null
  -s f0.txt
  --debug-config=config.out)

config_test_debug_pass(BasicSourceOptions_01_w)

# Compare output from BasicSourceOptions_01_w against reference.
config_ref_test(BasicSourceOptions_01)

cet_test(BasicSourceOptions_02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c BasicSourceOptions_02.fcl
  -s f0.txt
  --debug-config=config.out
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "Error: source files specified for EmptyEvent source\\."
  DATAFILES fcl/BasicSourceOptions_02.fcl)

cet_test(BasicSourceOptions_03 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --estart 5 --nevts 4 --nskip 2
  -c /dev/null
  -s f0.txt
  -S BasicSourceOptions-srclist.txt
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "--source-list \\(-S\\) and --source \\(-s\\) or non-option arguments are incompatible due to ordering ambiguities\\."
  DATAFILES BasicSourceOptions-srclist.txt)

cet_test(BasicSourceOptions_04_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --estart 5 --nevts 4 --nskip 2
  -c BasicSourceOptions_04.fcl
  -S BasicSourceOptions-empty-srclist.txt
  --debug-config=config.out
  DATAFILES fcl/BasicSourceOptions_04.fcl
  BasicSourceOptions-empty-srclist.txt)

config_test_debug_pass(BasicSourceOptions_04_w)

# Compare output from BasicSourceOptions_04_w against reference.
config_ref_test(BasicSourceOptions_04)

cet_test(BasicSourceOptions_05_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --estart 5 --nevts 4 --nskip 2
  -c BasicSourceOptions_04.fcl
  -S BasicSourceOptions-srclist.txt
  --debug-config=config.out
  DATAFILES fcl/BasicSourceOptions_04.fcl
  BasicSourceOptions-srclist.txt)

config_test_debug_pass(BasicSourceOptions_05_w)

# Compare output from BasicSourceOptions_05_w against reference.
config_ref_test(BasicSourceOptions_05)

####################################
# Test empty paths
cet_test(EmptyPaths_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty-paths.fcl
  DATAFILES fcl/empty-paths.fcl)

cet_test(EmptyRepeatedPaths_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty-repeated-paths.fcl
  DATAFILES fcl/empty-repeated-paths.fcl)

####################################
# BasicPostProcessor.

# Should pass and provide config output file for comparison in
# BasicPostProcessor_01_r against reference.
cet_test(BasicPostProcessor_01_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c BasicPostProcessor_01_w.fcl
  --debug-config=config.out
  DATAFILES fcl/BasicPostProcessor_01_w.fcl)

config_test_debug_pass(BasicPostProcessor_01_w)

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
  -c /dev/null)

config_test_debug_pass(DebugOptions_01_w)

# Compare output from DebugOptions_01_w against reference.
config_ref_test(DebugOptions_01)

cet_test(DebugOptions_02_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --validate-config=/dev/null -c validate-config.fcl
  DATAFILES fcl/validate-config.fcl
  TEST_PROPERTIES WILL_FAIL TRUE)

# TimeTracker enabling
cet_test(DebugOptionsTimeTracker_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --timing --rethrow-default -c /dev/null)

cet_test(DebugOptionsTimeTracker_02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --timing-db=timing.db --rethrow-default -c /dev/null)

cet_test(DebugOptionsTimeTracker_03 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --timing-db=timing.db --no-timing --rethrow-default -c /dev/null
  TEST_PROPERTIES WILL_FAIL TRUE)

####################################
# FileCatalogOptionsHandler.

# Should fail due to requiring both main SAM options.
cet_test(FileCatalogOptions_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  -c /dev/null
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  PASS_REGULAR_EXPRESSION
  "  configurations services\\.CatalogInterface\\.webURI \\(--sam-web-uri\\) and
  services\\.FileCatalogMetadata\\.processID \\(--sam-process-id\\) must be specified
  together or not at all\\.")

# Should exit 1 after printing config -- no output means no metadata required.
cet_test(FileCatalogOptions_02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -c /dev/null
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  PASS_REGULAR_EXPRESSION "ART_DEBUG_CONFIG is defined"
  PASS_REGULAR_EXPRESSION "Config output follows")

# Should fail due to missing metadata.
cet_test(FileCatalogOptions_03 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c /dev/null
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  PASS_REGULAR_EXPRESSION
  "  Output \"out\" must be configured with dataTier \\(--sam-data-tier=out:<tier>\\) and streamName \\(--sam-stream-name=out:<stream>\\)\\.\n")

# Should fail due to missing / default process name.
cet_test(FileCatalogOptions_04 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c /dev/null
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "Non-empty / default process_name required for SAM metadata\\.")

# Should pass and provide config output file for comparison in
# FileCatalogOptions_05_r.
cet_test(FileCatalogOptions_05_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c /dev/null
  --process-name=SAMTEST
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  --debug-config=config.out)

config_test_debug_pass(FileCatalogOptions_05_w)

# Compare output from FileCatalogOptions_05_w against reference.
config_ref_test(FileCatalogOptions_05
  OUTPUT_FILTERS DEFAULT ${filterOutputModule})

# Should pass and provide config output file for comparison in
# FileCatalogOptions_06_r.
cet_test(FileCatalogOptions_06_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c /dev/null
  --process-name=SAMTEST
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  --sam-stream-name "wannasweetie"
  --debug-config=config.out)

config_test_debug_pass(FileCatalogOptions_06_w)

# Compare output from FileCatalogOptions_06_w against reference.
config_ref_test(FileCatalogOptions_06
  OUTPUT_FILTERS DEFAULT ${filterOutputModule})

# Should pass and provide config output file for comparison in
# FileCatalogOptions_07_r.
cet_test(FileCatalogOptions_07_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c /dev/null
  --process-name=SAMTEST
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  --sam-stream-name "out:wannabikkit"
  --sam-stream-name "wannasweetie"
  --debug-config=config.out)

config_test_debug_pass(FileCatalogOptions_07_w)

# Compare output from FileCatalogOptions_07_w against reference.
config_ref_test(FileCatalogOptions_07
  OUTPUT_FILTERS DEFAULT ${filterOutputModule})

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
  --debug-config=config.out
  DATAFILES fcl/simple-with-streamName.fcl)

config_test_debug_pass(FileCatalogOptions_08_w)

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
  --debug-config=config.out
  DATAFILES fcl/simple-with-streamName.fcl)

config_test_debug_pass(FileCatalogOptions_09_w)

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
  --debug-config=config.out
  DATAFILES fcl/simple-with-streamName.fcl)

config_test_debug_pass(FileCatalogOptions_10_w)

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
  --debug-config=config.out
  DATAFILES fcl/simple-with-dataTier.fcl)

config_test_debug_pass(FileCatalogOptions_11_w)

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
  --debug-config=config.out
  DATAFILES fcl/simple-with-dataTier.fcl)

config_test_debug_pass(FileCatalogOptions_12_w)

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
  --debug-config=config.out
  DATAFILES fcl/simple-with-dataTier.fcl)

config_test_debug_pass(FileCatalogOptions_13_w)

# Compare output from FileCatalogOptions_13_w against reference.
config_ref_test(FileCatalogOptions_13)

# Should pass and provide config output file for comparison in
# FileCatalogOptions_14_r.
cet_test(FileCatalogOptions_14_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c /dev/null
  --process-name=SAMTEST
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  --debug-config=config.out)

config_test_debug_pass(FileCatalogOptions_14_w)

# Compare output from FileCatalogOptions_14_w against reference.
config_ref_test(FileCatalogOptions_14
  OUTPUT_FILTERS DEFAULT ${filterOutputModule})

# Should all fail due to conflicting options
cet_test(FileCatalogOptions_15 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-inherit-file-type
  --sam-file-type "Chicken"
  -c /dev/null
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "  The options '--sam-inherit-file-type' and '--sam-file-type' are mutually exclusive\\.")

cet_test(FileCatalogOptions_16 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-inherit-run-type
  --sam-run-type "Turkey"
  -c /dev/null
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "  The options '--sam-inherit-run-type' and '--sam-run-type' are mutually exclusive\\.")

cet_test(FileCatalogOptions_17 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-inherit-metadata
  --sam-file-type "Duck"
  -c /dev/null
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "  The options '--sam-inherit-metadata' and '--sam-file-type' are mutually exclusive\\.")

cet_test(FileCatalogOptions_18 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-inherit-metadata
  --sam-run-type "Turducken"
  -c /dev/null
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "  The options '--sam-inherit-metadata' and '--sam-run-type' are mutually exclusive\\.")

cet_test(MaxThreadsSpecified_t1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c /dev/null --nthreads 2
  TEST_PROPERTIES ENVIRONMENT OMP_NUM_THREADS=1
  PASS_REGULAR_EXPRESSION
  "The specified number of threads \\(2\\) exceeds the allowed number \\(1\\).*The allowed number of threads \\(1\\) will be used for this job.")

cet_test(MaxThreadsSpecified_t2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c /dev/null -j 2
  TEST_PROPERTIES ENVIRONMENT OMP_NUM_THREADS=1
  PASS_REGULAR_EXPRESSION
  "The specified number of threads \\(2\\) exceeds the allowed number \\(1\\).*The allowed number of threads \\(1\\) will be used for this job.")

cet_test(MaxThreadsSpecified_t3 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c /dev/null -j 2 -M stdout
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "TBB has been configured to use.*a maximum of 2 threads")

cet_test(MaxThreadsSpecified_t4 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c /dev/null -j 2 -M stdout
  TEST_PROPERTIES ENVIRONMENT OMP_NUM_THREADS=3
  PASS_REGULAR_EXPRESSION
  "TBB has been configured to use.*a maximum of 2 threads")

cet_test(MaxThreadsSpecified_t5 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c /dev/null -j 0 -M stdout
  TEST_PROPERTIES ENVIRONMENT OMP_NUM_THREADS=3
  PASS_REGULAR_EXPRESSION
  "TBB has been configured to use.*a maximum of 3 threads")
