set(art_Framework_Services_Interfaces_sources
  FileDeliveryStatus.cc
  FileDisposition.cc
  FileTransferStatus.cc
  UserInteraction.cc
)

art_make_library( LIBRARY_NAME art_Framework_Services_Interfaces
                  SOURCE ${art_Framework_Services_Interfaces_sources}
                  LIBRARIES art_Framework_Core
  art_Framework_Principal
  art_Persistency_Provenance
  art_Framework_Services_Registry
  ${FHICLCPP}
)

install_headers(SUBDIRS detail)
install_source()
