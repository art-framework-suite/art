# cet_test macro
include(CetTest)

link_libraries(
  art_Framework_Services_Registry
  art_Utilities
  art_Framework_Core
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

cet_test(artapp_t USE_BOOST_UNIT
  LIBRARIES
  art_Framework_Art
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  DATAFILES
  empty_config.fcl
)
