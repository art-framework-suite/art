set(art_Framework_IO_Sources_sources
  detail/FileServiceProxy.cc
)

add_library(art_Framework_IO_Sources
  SHARED
  ${art_Framework_IO_Sources_sources}
)

target_link_libraries(art_Framework_IO_Sources
  art_Framework_Services_Interfaces
  art_Framework_Services_Registry
  art_Utilities
)

install (TARGETS art_Framework_IO_Sources
  DESTINATION ${flavorqual_dir}/lib )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
