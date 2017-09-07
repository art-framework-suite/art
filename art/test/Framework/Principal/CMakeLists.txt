# cet_test macro

art_dictionary(NO_INSTALL)

set(test_Framework_Principal_libraries
  art_Framework_Core
  art_Framework_Principal
  art_Utilities canvas
 )

cet_test(ClosedRangeSetHandler_t USE_BOOST_UNIT LIBRARIES ${test_Framework_Principal_libraries})
cet_test(Event_t USE_BOOST_UNIT LIBRARIES ${test_Framework_Principal_libraries})
cet_test(EventPrincipal_t USE_BOOST_UNIT LIBRARIES ${test_Framework_Principal_libraries})
cet_test(OpenRangeSetHandler_t USE_BOOST_UNIT LIBRARIES ${test_Framework_Principal_libraries})
