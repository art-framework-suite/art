art_make_library(LIBRARY_NAME art_Framework_IO_Catalog
  SOURCE InputFileCatalog.cc
  LIBRARIES PRIVATE
    art::art_Framework_Services_Optional_TrivialFileDelivery_service
    art::art_Framework_Services_Optional_TrivialFileTransfer_service
    art::art_Utilities
    Boost::filesystem
)

install_headers()
install_source()
