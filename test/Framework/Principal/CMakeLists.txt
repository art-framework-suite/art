# cet_test macro

art_dictionary(NO_INSTALL)

link_libraries(
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
  -ldl
 )

cet_test(eventprincipal_t USE_BOOST_UNIT)
cet_test(Event_t USE_BOOST_UNIT)
cet_test(GroupFactory_t USE_BOOST_UNIT)
