#include <cassert>

#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "ProdigalAnalyzer.h"
#include "test/TestObjects/ToyProducts.h"

namespace arttest {
  ProdigalAnalyzer::ProdigalAnalyzer(fhicl::ParameterSet const& )
  {
  }

  void ProdigalAnalyzer::analyze(art::Event const& e, art::EventSetup const&) {
    art::Handle<Prodigal> h;
    assert(e.getByLabel("maker", h));
    assert(h.provenance()->parents().empty());
  }

}
using arttest::ProdigalAnalyzer;
DEFINE_ART_MODULE(ProdigalAnalyzer);
