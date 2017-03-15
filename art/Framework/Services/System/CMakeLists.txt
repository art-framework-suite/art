simple_plugin(CurrentModule "service" art_Persistency_Provenance canvas)
simple_plugin(DatabaseConnection "service")
simple_plugin(FileCatalogMetadata "service")
simple_plugin(FloatingPointControl "service")
simple_plugin(ScheduleContext "service" art_Framework_Core)
simple_plugin(TriggerNamesService "service")

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
