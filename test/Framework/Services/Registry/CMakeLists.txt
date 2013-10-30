include(CetTest)

cet_test(GlobalSignal_t USE_BOOST_UNIT
  LIBRARIES
  -ldl
  )

cet_test(LocalSignal_t USE_BOOST_UNIT
  LIBRARIES
  -ldl
  )
