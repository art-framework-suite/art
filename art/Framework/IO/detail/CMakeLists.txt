art_make_library(
  LIBRARY_NAME art_Framework_IO_detail
  SOURCE
    FileNameComponents.cc
    logFileAction.cc
    validateFileNamePattern.cc
  LIBRARIES PRIVATE
    Boost::regex
    messagefacility::MF_MessageLogger
    canvas::canvas
)

install_headers()
install_source()
