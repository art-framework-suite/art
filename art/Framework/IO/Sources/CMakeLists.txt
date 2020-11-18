art_make_library(
  LIBRARY_NAME
    art_Framework_IO_Sources
  SOURCE
    detail/FileServiceProxy.cc
    SourceHelper.cc
  LIBRARIES
    art::art_Framework_Services_FileServiceInterfaces
    art::art_Framework_Principal
  PRIVATE
    art::art_Utilities
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
