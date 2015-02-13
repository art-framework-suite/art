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

art_make_library( LIBRARY_NAME art_Framework_Principal
                  SOURCE ${art_Framework_Principal_sources}
                  LIBRARIES art_Persistency_Provenance
  art_Persistency_Common
  art_Framework_Services_Registry
  art_Utilities
  art_Version
  MF_MessageLogger
  fhiclcpp
  cetlib
  ${CLHEP}
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
