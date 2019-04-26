art_make_library(
  LIBRARY_NAME art_Framework_IO_detail
  SOURCE
  FileNameComponents.cc
  logFileAction.cc
  validateFileNamePattern.cc
  LIBRARIES
  Boost::regex
  MF_MessageLogger
  canvas
  cetlib_except
  )

install_headers()
install_source()
