#include <cassert>

#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/Utilities/Exception.h"
#include "art/MessageLogger/MessageLogger.h"
#include "ProdigalAnalyzer.h"
#include "test/TestObjects/ToyProducts.h"

namespace edmtest {
  ProdigalAnalyzer::ProdigalAnalyzer(edm::ParameterSet const& )
  {
  }

  void ProdigalAnalyzer::analyze(edm::Event const& e, edm::EventSetup const&) {
    edm::Handle<Prodigal> h;
    assert(e.getByLabel("maker", h));
    assert(h.provenance()->parents().empty());
  }

}
using edmtest::ProdigalAnalyzer;
DEFINE_FWK_MODULE(ProdigalAnalyzer);
