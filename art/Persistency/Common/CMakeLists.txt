cet_make_library(SOURCE
    DelayedReader.cc
    GroupQueryResult.cc
  LIBRARIES
  PUBLIC
    canvas::canvas
    cetlib::cetlib
    cetlib_except::cetlib_except
)

install_headers()
install_source()
