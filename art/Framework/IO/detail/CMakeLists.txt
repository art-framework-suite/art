art_make_library(
  LIBRARY_NAME art_Framework_IO_detail
  SOURCE
  FileNameComponents.cc
  logFileAction.cc
  validateFileNamePattern.cc
  LIBRARIES
  ${Boost_REGEX_LIBRARY}
  MF_MessageLogger
  MF_Utilities
  canvas
  cetlib_except
  )

install_headers()
install_source()
