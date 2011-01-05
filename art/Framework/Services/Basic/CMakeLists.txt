# project name
project(art_Framework_Services_Basic)

simple_plugin(ConstProductRegistry "service")
simple_plugin(CurrentModule "service")
simple_plugin(FloatingPointControl "service")
simple_plugin(RandomNumberGenerator "service")
simple_plugin(SimpleMemoryCheck "service")
simple_plugin(TFileService "service")
simple_plugin(Timing "service")
simple_plugin(Tracer "service")
simple_plugin(TriggerNamesService "service")

install(FILES
  ConstProductRegistry.h
  CurrentModule.h
  FloatingPointControl.h
  RandomNumberGenerator.h
  TFileService.h
  TriggerNamesService.h
  DESTINATION ${product}/${version}/include/art/Framework/Services/Basic
)
