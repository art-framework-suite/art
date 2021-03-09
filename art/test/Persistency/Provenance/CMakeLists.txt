cet_test(ProcessHistoryRegistry_t USE_BOOST_UNIT
  LIBRARIES PRIVATE
  art::Persistency_Provenance
  art::Version
  fhiclcpp::fhiclcpp
  cetlib::container_algorithms
  hep_concurrency::simultaneous_function_spawner
)

cet_test(ModuleDescription_t LIBRARIES PRIVATE art::Persistency_Provenance)
