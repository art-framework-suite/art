art_dictionary()

simple_plugin(BlockingPrescaler       "module")
simple_plugin(DataFlowDumper          "module")
simple_plugin(EmptyEvent              "source")
simple_plugin(EventIDFilter           "module")
simple_plugin(FileDumperOutput        "module")
simple_plugin(Prescaler               "module")
simple_plugin(ProvenanceCheckerOutput "module")
simple_plugin(RandomNumberSaver       "module")

simple_plugin(SamplingInput "source"
  art_Framework_IO_Root
  ${ROOT_RIO}
  ${ROOT_TREE}
  SOURCE
  SamplingInput_source.cc
  detail/DataSetSampler.cc
  detail/SamplingDelayedReader.cc
  detail/SamplingInputFile.cc)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
