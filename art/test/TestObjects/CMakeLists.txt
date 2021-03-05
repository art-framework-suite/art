cet_make_library(INTERFACE
  SOURCE
    AssnTestData.h
    MockCluster.h
    ProductWithPtrs.h
    ToyProducts.h
  LIBRARIES INTERFACE canvas::canvas
)

install_headers()
