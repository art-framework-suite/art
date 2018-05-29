find_package(ROOT REQUIRED COMPONENTS Hist Core)

art_make_library(SOURCE TH1Data.cc
  LIBRARIES ${ROOT_Hist_LIBRARY} ${ROOT_Core_LIBRARY}
  )

art_dictionary(DICTIONARY_LIBRARIES art_test_TestObjects)

install_headers()
