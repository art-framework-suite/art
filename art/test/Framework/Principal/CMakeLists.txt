foreach(test IN ITEMS ClosedRangeSetHandler_t Event_t EventPrincipal_t OpenRangeSetHandler_t)
  cet_test(${test} USE_BOOST_UNIT
    LIBRARIES PRIVATE art::art_Framework_Principal art::art_Version
  )
endforeach()

cet_test(Selector_t USE_BOOST_UNIT LIBRARIES PRIVATE art::art_Framework_Principal)
