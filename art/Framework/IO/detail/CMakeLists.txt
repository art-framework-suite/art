art_make_library(
  LIBRARY_NAME art_Framework_IO_detail
  SOURCE logFileAction.cc
  LIBRARIES MF_MessageLogger
  )

install_headers()
install_source()
