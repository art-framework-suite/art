
art_make_library( LIBRARY_NAME art_Framework_IO_ProductMix
                  SOURCE MixHelper.cc ProdToProdMapBuilder.cc
                  LIBRARIES art_Framework_IO_Root
  art_Framework_Services_System_CurrentModule_service
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_Optional_RandomNumberGenerator_service
  art_Framework_Core
  art_Framework_Principal
  art_Framework_Services_Registry
  art_Persistency_Common
  art_Persistency_Provenance canvas_Persistency_Provenance
  art_Utilities canvas_Utilities
  MF_MessageLogger
  fhiclcpp
  cetlib
  )

art_dictionary(DICTIONARY_LIBRARIES art_Persistency_Provenance canvas_Persistency_Provenance)

install_headers()
install_source()
