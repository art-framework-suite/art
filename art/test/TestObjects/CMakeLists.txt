art_make_library(SOURCE TH1Data.cc
  LIBRARIES ${ROOT_HIST} ${ROOT_CORE}
  )

art_dictionary(DICTIONARY_LIBRARIES art_test_TestObjects canvas_Persistency_Common_dict)

install_headers()
