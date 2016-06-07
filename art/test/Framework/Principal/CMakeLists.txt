# cet_test macro

art_dictionary(NO_INSTALL)

set( test_Framework_Principal_libraries
  art_Framework_Core
  art_Framework_Principal
  art_Utilities canvas_Utilities
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

cet_test(eventprincipal_t USE_BOOST_UNIT LIBRARIES ${test_Framework_Principal_libraries})
cet_test(Event_t USE_BOOST_UNIT LIBRARIES ${test_Framework_Principal_libraries})
cet_test(GroupFactory_t USE_BOOST_UNIT LIBRARIES ${test_Framework_Principal_libraries})
cet_test(ClosedRangeSetHandler_t USE_BOOST_UNIT LIBRARIES ${test_Framework_Principal_libraries})
cet_test(OpenRangeSetHandler_t USE_BOOST_UNIT LIBRARIES ${test_Framework_Principal_libraries})
