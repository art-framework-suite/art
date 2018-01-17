art_make_library(LIBRARY_NAME art_Framework_IO_ProductMix
  SOURCE
  MixHelper.cc
  MixOpBase.cc
  ProdToProdMapBuilder.cc
  detail/checkForMissingDictionaries.cc
  LIBRARIES
  art_Framework_IO_Root
  art_Framework_Services_System_CurrentModule_service
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_Optional_RandomNumberGenerator_service
  art_Framework_Core
  art_Framework_Principal
  art_Framework_Services_Registry
  art_Persistency_Common
  art_Persistency_Provenance canvas
  art_Utilities
  MF_MessageLogger
  fhiclcpp
  canvas
  canvas_root_io
  cetlib)

if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
  set_property(SOURCE MixHelper.cc APPEND
    PROPERTY COMPILE_DEFINITIONS _LIBCPP_ENABLE_CXX17_REMOVED_FEATURES=1
    )
endif()

art_dictionary(DICTIONARY_LIBRARIES art_Persistency_Provenance canvas)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
