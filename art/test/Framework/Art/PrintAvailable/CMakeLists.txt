cet_build_plugin(DummyProducer art::Producer NO_INSTALL)
cet_build_plugin(DummyFilter art::Filter NO_INSTALL)
cet_build_plugin(DummyAnalyzer art::Analyzer NO_INSTALL)
cet_build_plugin(DummyResultsProducer art::ResultsProducer NO_INSTALL)
cet_build_plugin(DummyService art::service NO_INSTALL)
cet_build_plugin(DummySource art::InputSource NO_INSTALL
  LIBRARIES REG
    art::Framework_Principal
    fhiclcpp::types
)

cet_build_plugin(DummyClassTool art::tool NO_INSTALL BASENAME_ONLY)
cet_build_plugin(DummyFunctionTool art::tool NO_INSTALL BASENAME_ONLY)

add_subdirectory(Colliding)
