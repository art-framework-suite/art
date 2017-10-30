art_make_library(
  LIBRARY_NAME art_Framework_IO_detail
  SOURCE logFileAction.cc validateFileNamePattern.cc
  LIBRARIES MF_MessageLogger canvas cetlib_except
  )

install_headers()
install_source()
