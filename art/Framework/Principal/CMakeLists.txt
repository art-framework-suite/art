set(art_Framework_Principal_sources
  Actions.cc
  AssnsGroup.cc
  CurrentProcessingContext.cc
  DataViewImpl.cc
  DeferredProductGetter.cc
  Event.cc
  EventPrincipal.cc
  Group.cc
  GroupFactory.cc
  NoDelayedReader.cc
  Principal.cc
  Provenance.cc
  Run.cc
  RunPrincipal.cc
  Selector.cc
  SelectorBase.cc
  SubRun.cc
  SubRunPrincipal.cc
  Worker.cc
)

add_library(art_Framework_Principal SHARED ${art_Framework_Principal_sources})
target_link_libraries(art_Framework_Principal
  art_Persistency_Provenance
  art_Persistency_Common
  art_Framework_Services_Registry
  art_Utilities
  art_Version
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
  ${CLHEP}
  ${SIGC}
)

install(TARGETS art_Framework_Principal
  DESTINATION
  ${flavorqual_dir}/lib
  )

install_headers()
install_source()
