art_make(LIB_LIBRARIES
  art_Framework_Services_System_ScheduleContext_service
  art_Framework_Services_Registry
  art_Persistency_Common
  art_Persistency_Provenance
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
