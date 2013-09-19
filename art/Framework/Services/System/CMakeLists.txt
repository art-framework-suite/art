# project name
project(art_Framework_Services_System)

simple_plugin(CurrentModule "service" art_Persistency_Provenance)
simple_plugin(FileCatalogMetadata "service")
simple_plugin(FloatingPointControl "service")
simple_plugin(PathSelection "service" art_Framework_Core)
simple_plugin(ScheduleContext "service")
simple_plugin(TriggerNamesService "service")

install_headers()
install_source()
