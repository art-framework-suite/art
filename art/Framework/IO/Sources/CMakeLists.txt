cet_make_library(SOURCE
    detail/FileServiceProxy.cc
    SourceHelper.cc
  LIBRARIES
  PUBLIC
    art::Framework_Principal
    art_plugin_types::FileDeliveryService
    art_plugin_types::FileTransferService
    art::Framework_Services_FileServiceInterfaces
    art::Framework_Services_Registry
    art::Persistency_Provenance
    canvas::canvas
    cetlib::headers
)

cet_make_library(LIBRARY_NAME Source INTERFACE
  EXPORT_SET PluginTypes SOURCE Source.h
  LIBRARIES INTERFACE
    art_plugin_types::InputSource
    art::Framework_IO_Sources
    art::Framework_Core
    art::Framework_Principal
    art::Persistency_Provenance
    canvas::canvas
    fhiclcpp::types
    fhiclcpp::fhiclcpp
    cetlib::headers
)  

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
