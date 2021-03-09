cet_make_library(SOURCE InputFileCatalog.cc
  LIBRARIES
  PUBLIC
    art::Framework_Services_FileServiceInterfaces
    art::Framework_Services_Registry
    fhiclcpp::types
  PRIVATE
    canvas::canvas
    fhiclcpp::fhiclcpp
    Boost::headers
)

install_headers()
install_source()
