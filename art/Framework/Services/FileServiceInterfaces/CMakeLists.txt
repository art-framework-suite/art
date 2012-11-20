set(art_Framework_Services_FileServiceInterfaces_sources
  FileDeliveryStatus.cc
  FileDisposition.cc
  FileTransferStatus.cc
)

art_make_library(LIBRARY_NAME art_Framework_Services_FileServiceInterfaces
  SOURCE ${art_Framework_Services_FileServiceInterfaces_sources}
  LIBRARIES art_Framework_Services_Registry
  ${FHICLCPP}
  )

install_headers(SUBDIRS detail)
install_source()
