cet_make_library(SOURCE
    Actions.cc
    ClosedRangeSetHandler.cc
    ConsumesInfo.cc
    DataViewImpl.cc
    Event.cc
    EventPrincipal.cc
    Group.cc
    NoDelayedReader.cc
    OpenRangeSetHandler.cc
    OutputHandle.cc
    Principal.cc
    ProcessTag.cc
    ProductInfo.cc
    Provenance.cc
    RangeSetHandler.cc
    Results.cc
    ResultsPrincipal.cc
    Run.cc
    RunPrincipal.cc
    Selector.cc
    SubRun.cc
    SubRunPrincipal.cc
    Worker.cc
  LIBRARIES
  PUBLIC
    art::Persistency_Provenance
    art::Persistency_Common
    hep_concurrency::hep_concurrency
  PRIVATE
    art::Framework_Services_Registry
    messagefacility::MF_MessageLogger
    Range::Range
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
