include(CetTest)

link_libraries(
  ${SIGC}
  -ldl
)

cet_test(GlobalSignal_t USE_BOOST_UNIT)
