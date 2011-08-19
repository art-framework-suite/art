# cet_test macro
include(CetTest)

art_dictionary()

link_libraries(
  art_Framework_Core
  art_Framework_Principal
  art_Utilities
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
  ${CINTEX}
  ${TREE}
  ${HIST}
  ${MATRIX}
  ${NET}
  ${MATHCORE}
  ${THREAD}
  ${RIO}
	${CORE}
  ${CINT}
	${REFLEX}
  ${CPPUNIT}
  -ldl
 )

cet_test(eventprincipal_t USE_BOOST_UNIT)
cet_test(Event_t USE_BOOST_UNIT)
cet_test(GroupFactory_t USE_BOOST_UNIT)
