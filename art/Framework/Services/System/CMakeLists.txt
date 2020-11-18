simple_plugin(DatabaseConnection "service")
simple_plugin(FileCatalogMetadata "service")
simple_plugin(FloatingPointControl "service"
  LIBRARIES PRIVATE
    hep_concurrency::hep_concurrency
    messagefacility::MF_MessageLogger
  SOURCE
    FloatingPointControl_service.cc
    detail/fpControl.cc)
simple_plugin(TriggerNamesService "service")

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
