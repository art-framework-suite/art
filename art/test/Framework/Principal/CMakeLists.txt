set(test_Framework_Principal_libraries
  art::art_Framework_Principal
  art::art_Version)

cet_test(ClosedRangeSetHandler_t USE_BOOST_UNIT LIBRARIES PRIVATE ${test_Framework_Principal_libraries})
cet_test(Event_t USE_BOOST_UNIT LIBRARIES PRIVATE ${test_Framework_Principal_libraries} messagefacility::MF_MessageLogger)
cet_test(EventPrincipal_t USE_BOOST_UNIT LIBRARIES PRIVATE ${test_Framework_Principal_libraries})
cet_test(OpenRangeSetHandler_t USE_BOOST_UNIT LIBRARIES PRIVATE ${test_Framework_Principal_libraries})
cet_test(Selector_t USE_BOOST_UNIT LIBRARIES PRIVATE art::art_Framework_Principal)
