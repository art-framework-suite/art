art_make_library(
  SOURCE
    FileDeliveryStatus.cc
    FileDisposition.cc
    FileTransferStatus.cc
  LIBRARIES
    art::art_Framework_Services_Registry
    fhiclcpp::fhiclcpp
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
