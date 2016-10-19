if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  cet_make_exec( ProcTester
    SOURCE ProcTester.cc
    LIBRARIES
    art_Framework_Services_Optional
    )

  cet_test( Proc_t HANDBUILT
    TEST_EXEC ProcTester
    )
endif()
