cet_make_library(SOURCE
  FileDeliveryStatus.cc
  FileDisposition.cc
  FileTransferStatus.cc
)

cet_make_library(LIBRARY_NAME FileDeliveryService INTERFACE
  EXPORT_SET ServicePluginType SOURCE CatalogInterface.h
  LIBRARIES INTERFACE
    art_plugin_types::serviceDeclaration
    art::Framework_Services_FileServiceInterfaces
    fhiclcpp::headers
    hep_concurrency::macros
)
make_simple_builder(art::FileDeliveryService)

cet_make_library(LIBRARY_NAME FileTransferService INTERFACE
   EXPORT_SET ServicePluginType SOURCE FileTransfer.h
   LIBRARIES INTERFACE art_plugin_types::serviceDeclaration)
make_simple_builder(art::FileTransferService)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
