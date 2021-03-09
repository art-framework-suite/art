cet_make_library(LIBRARY_NAME MixFilter INTERFACE
  EXPORT_SET PluginTypes SOURCE MixFilter.h
  LIBRARIES INTERFACE
    art::Framework_IO_ProductMix
    art::Framework_Core
    canvas::canvas
    fhiclcpp::types
    cetlib::cetlib
)

cet_make_library(LIBRARY_NAME ProvenanceDumperOutput INTERFACE
  EXPORT_SET PluginTypes SOURCE ProvenanceDumper.h
  LIBRARIES INTERFACE
    art::Framework_Core
    art::Framework_Principal
    canvas::canvas
    fhiclcpp::types
    fhiclcpp::fhiclcpp
    cetlib::cetlib
)

build_plugin(BlockingPrescaler "module" LIBRARIES REG fhiclcpp::types)

build_plugin(DataFlowDumper "module" BASE ProvenanceDumperOutput)

build_plugin(EmptyEvent "source" BASE DRISISource
  LIBRARIES REG
    art::Framework_Core
    art::Framework_Principal
    art::Persistency_Provenance
    canvas::canvas
    fhiclcpp::types
    cetlib::cetlib
)

build_plugin(EventIDFilter "module" LIBRARIES REG
    art::Framework_Core
    art::Framework_Principal
    canvas::canvas
    fhiclcpp::types
)

build_plugin(FileDumperOutput "module" LIBRARIES REG
    art::Framework_Principal
    canvas::canvas
    fhiclcpp::types
    cetlib::cetlib
)

build_plugin(Prescaler "module" LIBRARIES REG fhiclcpp::types)

build_plugin(ProvenanceCheckerOutput "module" LIBRARIES REG
    art::Framework_Principal
    canvas::canvas
    messagefacility::MF_MessageLogger
    fhiclcpp::types
    cetlib_except::cetlib_except
)

build_plugin(RandomNumberSaver "module" LIBRARIES REG
    art::Framework_Principal
    art::Framework_Services_Optional_RandomNumberGenerator_service
    fhiclcpp::types
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
