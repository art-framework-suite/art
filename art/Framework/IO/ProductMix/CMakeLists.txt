art_make_library(
  LIBRARY_NAME
    art_Framework_IO_ProductMix
  SOURCE
    MixHelper.cc
    ProdToProdMapBuilder.cc
  LIBRARIES
    art::art_Framework_Core
    art::art_Framework_Principal
    art::art_Framework_Services_Registry
  PRIVATE
    art::art_Framework_Services_System_TriggerNamesService_service
    art::art_Framework_Services_Optional_RandomNumberGenerator_service
    messagefacility::MF_MessageLogger
)

install_headers()
install_source()
