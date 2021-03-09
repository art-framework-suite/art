cet_make_library(SOURCE
    FileNameComponents.cc
    logFileAction.cc
    validateFileNamePattern.cc
  LIBRARIES
  PRIVATE
    fhiclcpp::types
    Boost::regex
    messagefacility::MF_MessageLogger
    canvas::canvas
)

install_headers()
install_source()
