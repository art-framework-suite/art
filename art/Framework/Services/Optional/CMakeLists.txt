set( art_Framework_Services_Optional_sources
  detail/TFileDirectory.cc
  detail/TH1AddDirectorySentry.cc
)

add_library(art_Framework_Services_Optional
  SHARED
  ${art_Framework_Services_Optional_sources}
)

install ( TARGETS art_Framework_Services_Optional
  DESTINATION ${flavorqual_dir}/lib )

target_link_libraries(art_Framework_Services_Optional
  ${CETLIB}
)

simple_plugin(RandomNumberGenerator "service")
simple_plugin(SimpleMemoryCheck "service")
simple_plugin(TFileService "service")
simple_plugin(Timing "service")
simple_plugin(Tracer "service")

install_headers(SUBDIRS detail)
install_source()
