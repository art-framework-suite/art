cet_test(ProcessHistoryRegistry_t USE_BOOST_UNIT
  LIBRARIES PRIVATE
  art::art_Persistency_Provenance
  art::art_Version
  Threads::Threads)

cet_test(ModuleDescription_t LIBRARIES PRIVATE art::art_Persistency_Provenance)
