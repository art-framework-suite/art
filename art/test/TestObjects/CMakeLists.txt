cet_make_library(
  LIBRARY_NAME art_test_TestObjects INTERFACE
  SOURCE
    AssnTestData.h
    MockCluster.h
    ProductWithPtrs.h
    ToyProducts.h
  LIBRARIES INTERFACE
    canvas::canvas
  )

install_headers()
