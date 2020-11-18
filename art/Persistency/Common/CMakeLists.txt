art_make_library(
  LIBRARY_NAME
    art_Persistency_Common
  SOURCE
    DelayedReader.cc
    GroupQueryResult.cc
  LIBRARIES
    canvas::canvas
  )

install_headers()
install_source()
