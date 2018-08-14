art_make_library(LIBRARY_NAME art_Framework_IO_ProductMix
  SOURCE
  MixHelper.cc
  ProdToProdMapBuilder.cc
  LIBRARIES
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_Optional_RandomNumberGenerator_service
  art_Framework_Core
  art_Framework_Principal
  art_Framework_Services_Registry
  art_Persistency_Common
  art_Persistency_Provenance canvas
  MF_MessageLogger
  fhiclcpp
  canvas
  cetlib)
