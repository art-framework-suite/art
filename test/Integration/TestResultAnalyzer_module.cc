#include "art/Framework/Principal/CurrentProcessingContext.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/TriggerResults.h"
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
   virtual ~TestResultAnalyzer();

   virtual void analyze(art::Event const& e);
   void endJob();

private:
   int    passed_;
   int    failed_;
   bool   dump_;
   int    numbits_;
   std::string expected_pathname_; // if empty, we don't know
   std::string expected_modulelabel_; // if empty, we don't know
};

arttest::TestResultAnalyzer::TestResultAnalyzer(fhicl::ParameterSet const& ps):
   art::EDAnalyzer(ps),
   passed_(),
   failed_(),
   dump_(ps.get<bool>("dump",false)),
   numbits_(ps.get<int>("numbits",-1)),
   expected_pathname_(ps.get<std::string>("pathname", "")),
   expected_modulelabel_(ps.get<std::string>("modlabel", ""))
{
}

arttest::TestResultAnalyzer::~TestResultAnalyzer()
{
}

void arttest::TestResultAnalyzer::analyze(art::Event const& e) {
   typedef std::vector<art::Handle<art::TriggerResults> > Trig;
   Trig prod;
   e.getManyByType(prod);

   art::CurrentProcessingContext const* cpc = currentContext();
   assert( cpc != 0 );
   assert( cpc->moduleDescription() != 0 );

   if ( !expected_pathname_.empty() )
      assert( expected_pathname_ == *(cpc->pathName()) );

   if ( !expected_modulelabel_.empty() )
      {
         assert(expected_modulelabel_ == *(cpc->moduleLabel()) );
      }

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
