
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/ParameterSet/ParameterSet.h"

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

extern "C"
{
  int a_trick_for_test_filters = 0;
}


namespace edmtest
{

  class TestFilterModule : public art::EDFilter
  {
  public:
    explicit TestFilterModule(art::ParameterSet const&);
    virtual ~TestFilterModule();

    virtual bool filter(art::Event& e, art::EventSetup const& c);
    void endJob();

  private:
    int count_;
    int accept_rate_; // how many out of 100 will be accepted?
    bool onlyOne_;
  };

  // -------

  class SewerModule : public art::OutputModule
  {
  public:
    explicit SewerModule(art::ParameterSet const&);
    virtual ~SewerModule();

  private:
    virtual void write(art::EventPrincipal const& e);
    virtual void writeSubRun(art::SubRunPrincipal const&){}
    virtual void writeRun(art::RunPrincipal const&){}
    virtual void endJob();

    std::string name_;
    int num_pass_;
    int total_;
  };

  // -----------------------------------------------------------------

  TestFilterModule::TestFilterModule(art::ParameterSet const& ps):
    count_(),
    accept_rate_(ps.getUntrackedParameter<int>("acceptValue",1)),
    onlyOne_(ps.getUntrackedParameter<bool>("onlyOne",false))
  {
  }

  TestFilterModule::~TestFilterModule()
  {
  }

  bool TestFilterModule::filter(art::Event&, art::EventSetup const&)
  {
    ++count_;
    assert( currentContext() != 0 );
    if(onlyOne_)
      return count_ % accept_rate_ ==0;
    else
      return count_ % 100 <= accept_rate_;
  }

  void TestFilterModule::endJob()
  {
    assert(currentContext() == 0);
  }

  // ---------

  SewerModule::SewerModule(art::ParameterSet const& ps):
    art::OutputModule(ps),
    name_(ps.getParameter<std::string>("name")),
    num_pass_(ps.getParameter<int>("shouldPass")),
    total_()
  {
  }

  SewerModule::~SewerModule()
  {
  }

  void SewerModule::write(art::EventPrincipal const&)
  {
    ++total_;
    assert(currentContext() != 0);
  }

  void SewerModule::endJob()
  {
    assert( currentContext() == 0 );
    std::cerr << "SEWERMODULE " << name_ << ": should pass " << num_pass_
	 << ", did pass " << total_ << "\n";

    if(total_!=num_pass_)
      {
	std::cerr << "number passed should be " << num_pass_
	     << ", but got " << total_ << "\n";
	abort();
      }
  }
}

using edmtest::TestFilterModule;
using edmtest::SewerModule;


DEFINE_FWK_MODULE(TestFilterModule);
DEFINE_FWK_MODULE(SewerModule);
