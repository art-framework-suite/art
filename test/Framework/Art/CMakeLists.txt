macro(config_ref_test TNAME)
  cet_test(${TNAME}_r HANDBUILT
    TEST_EXEC diff
    TEST_ARGS -u ${TNAME}-ref.txt
    ../${TNAME}_w.d/config.out
    DATAFILES ${TNAME}-ref.txt
    TEST_PROPERTIES DEPENDS ${TNAME}_w
    )
endmacro()

link_libraries(
  art_Framework_Services_Registry
  art_Utilities
  art_Framework_Core
  ${SIGC}
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
  empty_config.fcl
)

# Should fail due to requiring both main SAM options.
cet_test(FileCatalogOptions_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  DATAFILES fcl/opt-empty.fcl
)

SET_TESTS_PROPERTIES(FileCatalogOptions_01 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "--sam-web-uri and --sam-process-id must be used together or not at all."
)

# Should exit 1 after printing config -- no output means no metadata required.
cet_test(FileCatalogOptions_02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  DATAFILES fcl/opt-empty.fcl
)

SET_TESTS_PROPERTIES(FileCatalogOptions_02 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "ART_DEBUG_CONFIG is defined: config debug output follows"
)

# Should fail due to missing metadata.
cet_test(FileCatalogOptions_03 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --sam-web-uri=http://example.com
  --sam-process-id=778213
  -o out.root
  -c opt-empty.fcl
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=1
  DATAFILES fcl/opt-empty.fcl
)

SET_TESTS_PROPERTIES(FileCatalogOptions_03 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "SAM metadata information is required -- missing metadata:\n  --sam-application-family\n  --sam-application-version\n  --sam-file-type\n  --sam-data-tier\n"
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
  DATAFILES fcl/opt-empty.fcl
)

SET_TESTS_PROPERTIES(FileCatalogOptions_04 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "Non-empty / default process_name required for SAM metadata."
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

# Compare output from FileCatalogOptions_05_w against reference.
config_ref_test(FileCatalogOptions_05)

# Should pass and provide config output file for comparison in
# BasicSourceOptions_01_r against reference.
cet_test(BasicSourceOptions_01_w HANDBUILT
  TEST_EXEC art 
  TEST_ARGS --estart 5 --nevts 4 --nskip 2
  -c BasicSourceOptions_01_w.fcl
  -s f0.txt
  -S BasicSourceOptions_01-srclist.txt
  TEST_PROPERTIES ENVIRONMENT ART_DEBUG_CONFIG=config.out
  DATAFILES fcl/BasicSourceOptions_01_w.fcl
  BasicSourceOptions_01-srclist.txt
  )

# Compare output from BasicSourceOptions_01_w against reference.
config_ref_test(BasicSourceOptions_01)

SET_TESTS_PROPERTIES(FileCatalogOptions_05_w
  BasicSourceOptions_01_w
  PROPERTIES
  PASS_REGULAR_EXPRESSION
  "ART_DEBUG_CONFIG is defined: config debug output to file"
)
