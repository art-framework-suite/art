////////////////////////////////////////////////////////////////////////
// Class:       DummyResultsProducer
// Plugin Type: resultsproducer (art v2_06_03)
// File:        DummyResultsProducer_plugin.cc
//
// Generated at Fri May 19 10:15:57 2017 by Kyle Knoepfel using cetskelgen
// from cetlib version v2_03_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/ResultsProducer.h"

namespace fhicl {
  class ParameterSet;
}

namespace art {
  class Results;
  namespace test {
    class DummyResultsProducer;
  }
}


class art::test::DummyResultsProducer : public ResultsProducer {
public:
  explicit DummyResultsProducer(fhicl::ParameterSet const&) {}
  void clear() override {}
  void writeResults(Results&) override {}
};

DEFINE_ART_RESULTS_PLUGIN(art::test::DummyResultsProducer)
