#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>
#include <string>

namespace arttest {
   class TestResultAnalyzer;
}

class arttest::TestResultAnalyzer : public art::EDAnalyzer {
public:
   explicit TestResultAnalyzer(fhicl::ParameterSet const&);

   void analyze(art::Event const& e) override;
   void endJob() override;

private:
   int    passed_;
   int    failed_;
   bool   dump_;
   int    numbits_;
};

arttest::TestResultAnalyzer::TestResultAnalyzer(fhicl::ParameterSet const& ps):
   art::EDAnalyzer(ps),
   passed_(),
   failed_(),
   dump_(ps.get<bool>("dump",false)),
   numbits_(ps.get<int>("numbits",-1))
{
  consumesMany<art::TriggerResults>();
}

void arttest::TestResultAnalyzer::analyze(art::Event const& e) {
   typedef std::vector<art::Handle<art::TriggerResults> > Trig;
   Trig prod;
   e.getManyByType(prod);

   if(prod.size() == 0) return;
   if(prod.size() > 1) {
      mf::LogWarning("MultipleTriggerResults")
         << "More than one trigger result in the event, using first one.";
   }

   if (prod[0]->accept()) ++passed_; else ++failed_;

   if(numbits_ < 0) return;

   unsigned int numbits = numbits_;
   if(numbits != prod[0]->size()) {
      throw cet::exception("WrongNumberBits")
         << "Should have " << numbits
         << ", got " << prod[0]->size() << " in TriggerResults\n";
   }
}

void arttest::TestResultAnalyzer::endJob()
{
   mf::LogAbsolute("TestResultAnalyzerReport")
      << "passed=" << passed_ << " failed=" << failed_ << "\n";
}

DEFINE_ART_MODULE(arttest::TestResultAnalyzer)
