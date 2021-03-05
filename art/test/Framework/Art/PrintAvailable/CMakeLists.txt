build_plugin(DummyProducer "module" NO_INSTALL)
build_plugin(DummyFilter "module" NO_INSTALL)
build_plugin(DummyAnalyzer "module" NO_INSTALL)
build_plugin(DummyResultsProducer "plugin" BASE ResultsProducer NO_INSTALL)
build_plugin(DummyService "service" NO_INSTALL)
build_plugin(DummySource "source" BASE InputSource NO_INSTALL
  LIBRARIES REG
    art::Framework_Principal
    fhiclcpp::types
)

build_plugin(DummyClassTool "tool" NO_INSTALL BASENAME_ONLY)
build_plugin(DummyFunctionTool "tool" NO_INSTALL BASENAME_ONLY)

add_subdirectory(Colliding)
