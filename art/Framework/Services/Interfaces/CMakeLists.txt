set(art_Framework_Services_Interfaces_sources
  FileDeliveryStatus.cc
  FileDisposition.cc
  FileTransferStatus.cc
  UserInteraction.cc
)

add_library(art_Framework_Services_Interfaces
  SHARED
  ${art_Framework_Services_Interfaces_sources}
)

target_link_libraries(art_Framework_Services_Interfaces
  art_Framework_Core
  art_Framework_Principal
  art_Persistency_Provenance
  art_Framework_Services_Registry
  ${FHICLCPP}
)

install ( TARGETS art_Framework_Services_Interfaces
  DESTINATION ${flavorqual_dir}/lib )

install_headers(SUBDIRS detail)
install_source()
