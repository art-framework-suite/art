# project name
project(art_Framework_Services_Optional)

set( art_Framework_Services_Optional_sources
  UserInteraction.cc
)

add_library(art_Framework_Services_Optional
  SHARED
  ${art_Framework_Services_Optional_sources}
)

install ( TARGETS art_Framework_Services_Optional
  DESTINATION ${flavorqual_dir}/lib )

target_link_libraries(art_Framework_Services_Optional
  art_Persistency_Provenance
  art_Framework_Core
  ${FHICLCPP}
)

simple_plugin(RandomNumberGenerator "service")
simple_plugin(SimpleInteraction "service" art_Framework_Services_Optional)
simple_plugin(SimpleMemoryCheck "service")
simple_plugin(TFileService "service")
simple_plugin(Timing "service")
simple_plugin(Tracer "service")

install(FILES
  RandomNumberGenerator.h
  TFileService.h
  UserInteraction.h
  SimpleInteraction.h
  DESTINATION ${product}/${version}/include/art/Framework/Services/Optional
)
