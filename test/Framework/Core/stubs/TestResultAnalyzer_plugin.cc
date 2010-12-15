
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/OutputModule.h"
#include "fhiclcpp/ParameterSet.h"

#include "art/Framework/Core/Event.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/Framework/Core/CurrentProcessingContext.h"

#include <string>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <iterator>

using namespace art;

namespace arttest
{

  class TestResultAnalyzer : public art::EDAnalyzer
  {
  public:
    explicit TestResultAnalyzer(art::ParameterSet const&);
    virtual ~TestResultAnalyzer();

    virtual void analyze(art::Event const& e, art::EventSetup const& c);
    void endJob();

  private:
    int    passed_;
    int    failed_;
    bool   dump_;
    std::string name_;
    int    numbits_;
    std::string expected_pathname_; // if empty, we don't know
    std::string expected_modulelabel_; // if empty, we don't know
  };

  TestResultAnalyzer::TestResultAnalyzer(art::ParameterSet const& ps):
    passed_(),
    failed_(),
    dump_(ps.getUntrackedParameter<bool>("dump",false)),
    name_(ps.getUntrackedParameter<std::string>("name","DEFAULT")),
    numbits_(ps.getUntrackedParameter<int>("numbits",-1)),
    expected_pathname_(ps.getUntrackedParameter<std::string>("pathname", "")),
    expected_modulelabel_(ps.getUntrackedParameter<std::string>("modlabel", ""))
  {
  }

  TestResultAnalyzer::~TestResultAnalyzer()
  {
  }

  void TestResultAnalyzer::analyze(art::Event const& e,art::EventSetup const&)
  {
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
      std::cerr << "More than one trigger result in the event, using first one"
	   << std::endl;
    }

    if (prod[0]->accept()) ++passed_; else ++failed_;

    if(numbits_ < 0) return;

    unsigned int numbits = numbits_;
    if(numbits != prod[0]->size()) {
      std::cerr << "TestResultAnalyzer named: " << name_
	   << " should have " << numbits
	   << ", got " << prod[0]->size() << " in TriggerResults\n";
      abort();
    }
  }

  void TestResultAnalyzer::endJob()
  {
    std::cerr << "TESTRESULTANALYZER " << name_ << ": "
	 << "passed=" << passed_ << " failed=" << failed_ << "\n";
  }

}

using arttest::TestResultAnalyzer;

DEFINE_FWK_MODULE(TestResultAnalyzer);
