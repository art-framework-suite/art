
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

using namespace edm;

extern "C"
{
  int a_trick_for_test_filters = 0;
}


namespace edmtest
{

  class TestFilterModule : public edm::EDFilter
  {
  public:
    explicit TestFilterModule(edm::ParameterSet const&);
    virtual ~TestFilterModule();

    virtual bool filter(edm::Event& e, edm::EventSetup const& c);
    void endJob();

  private:
    int count_;
    int accept_rate_; // how many out of 100 will be accepted?
    bool onlyOne_;
  };

  // -------

  class SewerModule : public edm::OutputModule
  {
  public:
    explicit SewerModule(edm::ParameterSet const&);
    virtual ~SewerModule();

  private:
    virtual void write(edm::EventPrincipal const& e);
    virtual void writeLuminosityBlock(edm::LuminosityBlockPrincipal const&){}
    virtual void writeRun(edm::RunPrincipal const&){}
    virtual void endJob();

    std::string name_;
    int num_pass_;
    int total_;
  };

  // -----------------------------------------------------------------

  TestFilterModule::TestFilterModule(edm::ParameterSet const& ps):
    count_(),
    accept_rate_(ps.getUntrackedParameter<int>("acceptValue",1)),
    onlyOne_(ps.getUntrackedParameter<bool>("onlyOne",false))
  {
  }

  TestFilterModule::~TestFilterModule()
  {
  }

  bool TestFilterModule::filter(edm::Event&, edm::EventSetup const&)
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

  SewerModule::SewerModule(edm::ParameterSet const& ps):
    edm::OutputModule(ps),
    name_(ps.getParameter<std::string>("name")),
    num_pass_(ps.getParameter<int>("shouldPass")),
    total_()
  {
  }

  SewerModule::~SewerModule()
  {
  }

  void SewerModule::write(edm::EventPrincipal const&)
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
