#include "art/Framework/Principal/CurrentProcessingContext.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "cpp0x/algorithm"
#include "cpp0x/numeric"
#include "fhiclcpp/ParameterSet.h"
#include <iostream>
#include <iterator>
#include <string>

using namespace art;

extern "C"
{
  int a_trick_for_test_filters = 0;
}


namespace arttest
{

  class TestFilterModule : public art::EDFilter
  {
  public:
    explicit TestFilterModule(fhicl::ParameterSet const&);
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
    explicit SewerModule(fhicl::ParameterSet const&);
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

  TestFilterModule::TestFilterModule(fhicl::ParameterSet const& ps):
    count_(),
    accept_rate_(ps.get<int>("acceptValue",1)),
    onlyOne_(ps.get<bool>("onlyOne",false))
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

  SewerModule::SewerModule(fhicl::ParameterSet const& ps):
    art::OutputModule(ps),
    name_(ps.get<std::string>("name")),
    num_pass_(ps.get<int>("shouldPass")),
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

using arttest::TestFilterModule;
using arttest::SewerModule;


DEFINE_ART_MODULE(TestFilterModule);
DEFINE_ART_MODULE(SewerModule);
