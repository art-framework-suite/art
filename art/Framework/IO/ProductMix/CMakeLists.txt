
link_libraries(art_Framework_IO_Root
  art_Framework_Services_System_ConstProductRegistry_service
  art_Framework_Services_System_CurrentModule_service
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_Optional_RandomNumberGenerator_service
  art_Framework_Core
  art_Persistency_Common
  art_Persistency_Provenance
  art_Utilities
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
  )
add_library(art_Framework_IO_ProductMix
  SHARED
  MixHelper.cc
  ProdToProdMapBuilder.cc
  )
install ( TARGETS
  art_Framework_IO_ProductMix
  DESTINATION
  ${flavorqual_dir}/lib
  )

art_dictionary(DICTIONARY_LIBRARIES art_Persistency_Provenance)

install_headers()
install_source()
