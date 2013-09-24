foreach(ART_IPR_BASE_NAME FindOne FindMany)
  UNSET(ART_IPR_BY_PTR)
  SET(ART_IPR_CLASS_NAME ${ART_IPR_BASE_NAME})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${ART_IPR_BASE_NAME}.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${ART_IPR_CLASS_NAME}.h
    @ONLY FORCE)

  SET(ART_IPR_BY_PTR true)
  SET(ART_IPR_CLASS_NAME "${ART_IPR_BASE_NAME}P")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${ART_IPR_BASE_NAME}.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${ART_IPR_CLASS_NAME}.h
    @ONLY)
endforeach()

art_make(
  SUBDIRS detail
  LIB_LIBRARIES
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_System_FileCatalogMetadata_service
  art_Framework_Services_Optional_RandomNumberGenerator_service
  art_Framework_Principal
  art_Persistency_Common
  art_Persistency_Provenance
  art_Framework_Services_Registry
  art_Utilities
  art_Version
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
  ${CLHEP}
  ${SIGC}
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
