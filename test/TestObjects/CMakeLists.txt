art_make_library(SOURCE TH1Data.cc
  LIBRARIES ${ROOT_HIST} ${ROOT_CORE} NO_INSTALL
  )

art_dictionary(DICTIONARY_LIBRARIES
  test_TestObjects
  art_Persistency_Common_dict
  NO_INSTALL
  )
