# project name
project(art_Framework_Services_System)

simple_plugin(CurrentModule "service" art_Persistency_Provenance)
simple_plugin(FloatingPointControl "service")
simple_plugin(TriggerNamesService "service")

install(FILES
  CurrentModule.h
  FloatingPointControl.h
  TriggerNamesService.h
  DESTINATION ${product}/${version}/include/art/Framework/Services/System
)

install_source()
