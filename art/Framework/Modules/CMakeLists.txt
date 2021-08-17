include(art::module)
include(art::DRISISource)
include(art::Output)

cet_make_library(LIBRARY_NAME MixFilter INTERFACE
  EXPORT_SET PluginTypes SOURCE MixFilter.h
  LIBRARIES INTERFACE
    art::Framework_IO_ProductMix
    art::Framework_Core
    canvas::canvas
    fhiclcpp::types
    cetlib::cetlib
)
make_simple_builder(art::MixFilter BASE art::module)

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
make_simple_builder(art::ProvenanceDumperOutput BASE art::Output)

cet_build_plugin(BlockingPrescaler art::module LIBRARIES REG fhiclcpp::types)

include(art::ProvenanceDumperOutput)
cet_build_plugin(DataFlowDumper art::ProvenanceDumperOutput)

cet_build_plugin(EmptyEvent art::DRISISource
  LIBRARIES REG
    art::Framework_Core
    art::Framework_Principal
    art::Persistency_Provenance
    canvas::canvas
    fhiclcpp::types
    cetlib::cetlib
)

cet_build_plugin(EventIDFilter art::module LIBRARIES REG
    art::Framework_Core
    art::Framework_Principal
    canvas::canvas
    fhiclcpp::types
)

cet_build_plugin(FileDumperOutput art::module LIBRARIES REG
    art::Framework_Principal
    canvas::canvas
    fhiclcpp::types
    cetlib::cetlib
)

cet_build_plugin(Prescaler art::module LIBRARIES REG fhiclcpp::types)

cet_build_plugin(ProvenanceCheckerOutput art::module LIBRARIES REG
    art::Framework_Principal
    canvas::canvas
    messagefacility::MF_MessageLogger
    fhiclcpp::types
    cetlib_except::cetlib_except
)

cet_build_plugin(RandomNumberSaver art::module LIBRARIES REG
    art::Framework_Principal
    art::Framework_Services_Optional_RandomNumberGenerator_service
    fhiclcpp::types
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
