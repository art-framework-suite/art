# project name
project(art_Framework_Services_Optional)

simple_plugin(RandomNumberGenerator "service")
simple_plugin(SimpleMemoryCheck "service")
simple_plugin(TFileService "service")
simple_plugin(Timing "service")
simple_plugin(Tracer "service")

install(FILES
  RandomNumberGenerator.h
  TFileService.h
  DESTINATION ${product}/${version}/include/art/Framework/Services/Optional
)
