set(simple_tbb_tests
  tbb_init_t
  tbb_pfor_01_t
  tbb_preduce_01_t
)

foreach(tbb_test ${simple_tbb_tests})
  cet_test(${tbb_test}
    LIBRARIES ${TBB}
    )
endforeach()
