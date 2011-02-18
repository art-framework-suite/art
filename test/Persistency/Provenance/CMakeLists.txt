include (CetTest)

link_libraries( art_Persistency_Provenance
                art_Utilities
                art_Version
		${Boost_THREAD_LIBRARY}
    ${CINTEX}
    ${CORE}
    ${CINT}
		${REFLEX}
    -ldl
 )

cet_test(fileIndex_t SOURCES testRunner.cpp fileIndex_t.cppunit.cc
  LIBRARIES ${CPPUNIT}
  )

