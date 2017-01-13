simple_plugin(AssnsReaderTest "module" NO_INSTALL USE_BOOST_UNIT)

########################################################################
# Description of tests
#
# A: unsigned long
# B: std::string
# D: arttest::AssnsTestData
# 
#
# 1.
#   * Writer options:
#
#     dinkVoid: true
#
#   * Writer / reader options:
#
#     wantVoid: "ALL"
# 
#   * Event data products
#
#     PROCESS | MODULE_LABEL   | INSTANCE   | DATA PRODUCT TYPE   | SIZE
#     ASSNSW  | TriggerResults |            | art::TriggerResults |    -
#     ASSNSW  | ap1            |            | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap1            |            | art::Assns<A,B>     |    4
#     ASSNSW  | ap1            |            | std::vector<B>      |    4
#     ASSNSW  | ap1            |            | std::vector<A>      |    4
#     ASSNSW  | ap1            | many       | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap1            | many       | art::Assns<A,B>     |    6
#     ASSNSW  | ap1            | manymapvec | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap1            | manymapvec | art::Assns<A,B>     |    6
#     ASSNSW  | ap1            | mapvec     | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap1            | mapvec     | art::Assns<A,B>     |    4
#     ASSNSW  | ap1            | mv         | cet::map_vector<B>  |    4
#     ASSNSW  | ap2            |            | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap2            |            | art::Assns<A,B>     |    4
#     ASSNSW  | ap2            |            | std::vector<B>      |    4
#     ASSNSW  | ap2            |            | std::vector<A>      |    4
#     ASSNSW  | ap2            | many       | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap2            | many       | art::Assns<A,B>     |    6
#     ASSNSW  | ap2            | manymapvec | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap2            | manymapvec | art::Assns<A,B>     |    6
#     ASSNSW  | ap2            | mapvec     | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap2            | mapvec     | art::Assns<A,B>     |    4
#     ASSNSW  | ap2            | mv         | cet::map_vector<B>  |    4
#
#     Total products (present, not present): 23 (23, 0).
#
# 2.
#   * Writer options:
#
#     dinkVoid: true
#
#   * Writer / reader options:
#
#     wantVoid: "SOME"
# 
#   * Event data products
#
#     PROCESS | MODULE_LABEL   | INSTANCE   | DATA PRODUCT TYPE   | SIZE
#     ASSNSW  | TriggerResults |            | art::TriggerResults |    -
#     ASSNSW  | ap1            |            | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap1            |            | std::vector<B>      |    4
#     ASSNSW  | ap1            |            | std::vector<A>      |    4
#     ASSNSW  | ap1            | many       | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap1            | manymapvec | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap1            | manymapvec | art::Assns<A,B>     |    6
#     ASSNSW  | ap1            | mapvec     | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap1            | mapvec     | art::Assns<A,B>     |    4
#     ASSNSW  | ap1            | mv         | cet::map_vector<B>  |    4
#     ASSNSW  | ap2            |            | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap2            |            | std::vector<B>      |    4
#     ASSNSW  | ap2            |            | std::vector<A>      |    4
#     ASSNSW  | ap2            | many       | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap2            | manymapvec | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap2            | manymapvec | art::Assns<A,B>     |    6
#     ASSNSW  | ap2            | mapvec     | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap2            | mapvec     | art::Assns<A,B>     |    4
#     ASSNSW  | ap2            | mv         | cet::map_vector<B>  |    4
#     
#     Total products (present, not present): 19 (19, 0).
#     
# 3.
#   * Writer options:
#
#     dinkVoid: true
#
#   * Writer / reader options:
#
#     wantVoid: "NONE"
# 
#   * Event data products
#
#     PROCESS | MODULE_LABEL   | INSTANCE   | DATA PRODUCT TYPE   | SIZE
#     ASSNSW  | TriggerResults |            | art::TriggerResults |    -
#     ASSNSW  | ap1            |            | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap1            |            | std::vector<B>      |    4
#     ASSNSW  | ap1            |            | std::vector<A>      |    4
#     ASSNSW  | ap1            | many       | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap1            | manymapvec | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap1            | mapvec     | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap1            | mv         | cet::map_vector<B>  |    4
#     ASSNSW  | ap2            |            | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap2            |            | std::vector<B>      |    4
#     ASSNSW  | ap2            |            | std::vector<A>      |    4
#     ASSNSW  | ap2            | many       | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap2            | manymapvec | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap2            | mapvec     | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap2            | mv         | cet::map_vector<B>  |    4
#
#     Total products (present, not present): 15 (15, 0).
#
# 4.
#   * Writer options:
#
#     dinkVoid: true
#
#     source: { module_type: "RootInput" fileNames: [ "../AssnsReaderTest_w1.d/out.root" ] }
#
#   * Writer / reader options:
#
#     wantVoid: "NONE"
# 
#   * Event data products
#
#     PROCESS | MODULE_LABEL   | INSTANCE   | DATA PRODUCT TYPE   | SIZE
#     ASSNSW  | TriggerResults |            | art::TriggerResults |    -
#     ASSNSW  | ap1            |            | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap1            |            | art::Assns<A,B>     |    4
#     ASSNSW  | ap1            |            | std::vector<B>      |    4
#     ASSNSW  | ap1            |            | std::vector<A>      |    4
#     ASSNSW  | ap1            | many       | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap1            | many       | art::Assns<A,B>     |    6
#     ASSNSW  | ap1            | manymapvec | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap1            | manymapvec | art::Assns<A,B>     |    6
#     ASSNSW  | ap1            | mapvec     | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap1            | mapvec     | art::Assns<A,B>     |    4
#     ASSNSW  | ap1            | mv         | cet::map_vector<B>  |    4
#     ASSNSW  | ap2            |            | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap2            |            | art::Assns<A,B>     |    4
#     ASSNSW  | ap2            |            | std::vector<B>      |    4
#     ASSNSW  | ap2            |            | std::vector<A>      |    4
#     ASSNSW  | ap2            | many       | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap2            | many       | art::Assns<A,B>     |    6
#     ASSNSW  | ap2            | manymapvec | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap2            | manymapvec | art::Assns<A,B>     |    6
#     ASSNSW  | ap2            | mapvec     | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap2            | mapvec     | art::Assns<A,B>     |    4
#     ASSNSW  | ap2            | mv         | cet::map_vector<B>  |    4
#     ASSNSW2 | TriggerResults |            | art::TriggerResults |    -
#     ASSNSW2 | ap1            |            | art::Assns<A,B,D>   |    3
#     ASSNSW2 | ap1            |            | std::vector<B>      |    4
#     ASSNSW2 | ap1            |            | std::vector<A>      |    4
#     ASSNSW2 | ap1            | many       | art::Assns<A,B,D>   |    4
#     ASSNSW2 | ap1            | manymapvec | art::Assns<A,B,D>   |    4
#     ASSNSW2 | ap1            | mapvec     | art::Assns<A,B,D>   |    3
#     ASSNSW2 | ap1            | mv         | cet::map_vector<B>  |    4
#     ASSNSW2 | ap2            |            | art::Assns<A,B,D>   |    3
#     ASSNSW2 | ap2            |            | std::vector<B>      |    4
#     ASSNSW2 | ap2            |            | std::vector<A>      |    4
#     ASSNSW2 | ap2            | many       | art::Assns<A,B,D>   |    4
#     ASSNSW2 | ap2            | manymapvec | art::Assns<A,B,D>   |    4
#     ASSNSW2 | ap2            | mapvec     | art::Assns<A,B,D>   |    3
#     ASSNSW2 | ap2            | mv         | cet::map_vector<B>  |    4
#
#     Total products (present, not present): 38 (38, 0).
#
# 5.
#   * Writer options:
#
#     dinkVoid: true
#
#   * Writer / reader options:
#
#     wantVoid: "SOME"
#     wantMany: false
#     wantMV: false
#     wantAmbiguous: true
# 
#   * Event data products
#
#     PROCESS | MODULE_LABEL   | INSTANCE   | DATA PRODUCT TYPE   | SIZE
#     ASSNSW  | TriggerResults |            | art::TriggerResults |    -
#     ASSNSW  | ap1            |            | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap1            |            | art::Assns<A,B,B>   |    3
#     ASSNSW  | ap1            |            | std::vector<B>      |    4
#     ASSNSW  | ap1            |            | std::vector<A>      |    4
#     ASSNSW  | ap2            |            | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap2            |            | art::Assns<A,B,B>   |    3
#     ASSNSW  | ap2            |            | std::vector<B>      |    4
#     ASSNSW  | ap2            |            | std::vector<A>      |    4
#
#     Total products (present, not present): 9 (9, 0).
#
# 6.
#   * Writer options:
#
#     dinkVoid: true
#
#   * Writer / reader options:
#
#     wantVoid: "ALL"
#     wantAmbiguous: true
# 
#   * Event data products
#
#     PROCESS | MODULE_LABEL   | INSTANCE   | DATA PRODUCT TYPE   | SIZE
#     ASSNSW  | TriggerResults |            | art::TriggerResults |    -
#     ASSNSW  | ap1            |            | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap1            |            | art::Assns<A,B,B>   |    3
#     ASSNSW  | ap1            |            | art::Assns<A,B>     |    4
#     ASSNSW  | ap1            |            | std::vector<B>      |    4
#     ASSNSW  | ap1            |            | std::vector<A>      |    4
#     ASSNSW  | ap1            | many       | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap1            | many       | art::Assns<A,B>     |    6
#     ASSNSW  | ap1            | manymapvec | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap1            | manymapvec | art::Assns<A,B>     |    6
#     ASSNSW  | ap1            | mapvec     | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap1            | mapvec     | art::Assns<A,B>     |    4
#     ASSNSW  | ap1            | mv         | cet::map_vector<B>  |    4
#     ASSNSW  | ap2            |            | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap2            |            | art::Assns<A,B,B>   |    3
#     ASSNSW  | ap2            |            | art::Assns<A,B>     |    4
#     ASSNSW  | ap2            |            | std::vector<B>      |    4
#     ASSNSW  | ap2            |            | std::vector<A>      |    4
#     ASSNSW  | ap2            | many       | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap2            | many       | art::Assns<A,B>     |    6
#     ASSNSW  | ap2            | manymapvec | art::Assns<A,B,D>   |    4
#     ASSNSW  | ap2            | manymapvec | art::Assns<A,B>     |    6
#     ASSNSW  | ap2            | mapvec     | art::Assns<A,B,D>   |    3
#     ASSNSW  | ap2            | mapvec     | art::Assns<A,B>     |    4
#     ASSNSW  | ap2            | mv         | cet::map_vector<B>  |    4
#
#     Total products (present, not present): 25 (25, 0).
#
########################################################################
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
#    list(APPEND read_dfiles fcl/AssnsReaderTest_w.fcl)
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
