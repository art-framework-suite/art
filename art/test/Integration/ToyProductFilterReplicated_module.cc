// vim: set sw=2 expandtab :
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/ReplicatedFilter.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "fhiclcpp/ParameterSet.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace art;
using namespace std;

namespace {

  double
  f(int const val)
  {
    return sqrt(val);
  }

  void
  use_cpu_time(double& val)
  {
    for (int i = 0; i < 100'000'000; ++i) {
      val = f(i);
    }
  }

} // unnamed namespace

namespace arttest {

  class ToyProductFilterReplicated : public ReplicatedFilter {
  public:
    struct Config {
      fhicl::Atom<bool> outputModuleConfigured{
        fhicl::Name{"outputModuleConfigured"}};
    };
    using Parameters = Table<Config>;
    explicit ToyProductFilterReplicated(Parameters const& p, art::ScheduleID);
    ~ToyProductFilterReplicated();

  private:
    void beginJob(Services const&) override;
    void respondToOpenInputFile(FileBlock const&, Services const&) override;
    void respondToCloseInputFile(FileBlock const&, Services const&) override;
    void respondToOpenOutputFiles(FileBlock const&, Services const&) override;
    void respondToCloseOutputFiles(FileBlock const&, Services const&) override;
    void beginRun(Run const&, Services const&) override;
    void beginSubRun(SubRun const&, Services const&) override;
    bool filter(Event&, Services const&) override;
    void endSubRun(SubRun const&, Services const&) override;
    void endRun(Run const&, Services const&) override;
    void endJob(Services const&) override;

    bool const outputModuleConfigured_;
    bool beginJobCalled_{false};
    bool respondToOpenInputFileCalled_{false};
    bool respondToCloseInputFileCalled_{false};
    bool respondToOpenOutputFilesCalled_{false};
    bool respondToCloseOutputFilesCalled_{false};
    bool beginRunCalled_{false};
    bool beginSubRunCalled_{false};
    bool endSubRunCalled_{false};
    bool endRunCalled_{false};
    bool endJobCalled_{false};
  };

  ToyProductFilterReplicated::ToyProductFilterReplicated(
    Parameters const& p,
    art::ScheduleID const sid)
    : ReplicatedFilter{p, sid}
    , outputModuleConfigured_{p().outputModuleConfigured()}
  {}

  ToyProductFilterReplicated::~ToyProductFilterReplicated()
  {
    assert(beginJobCalled_);
    assert(respondToOpenInputFileCalled_);
    assert(respondToCloseInputFileCalled_);
    assert(respondToOpenOutputFilesCalled_ == outputModuleConfigured_);
    assert(respondToCloseOutputFilesCalled_ == outputModuleConfigured_);
    assert(beginRunCalled_);
    assert(beginSubRunCalled_);
    assert(endSubRunCalled_);
    assert(endRunCalled_);
    assert(endJobCalled_);
  }

  void
  ToyProductFilterReplicated::beginJob(Services const&)
  {
    beginJobCalled_ = true;
  }

  void
  ToyProductFilterReplicated::respondToOpenInputFile(FileBlock const&,
                                                     Services const&)
  {
    respondToOpenInputFileCalled_ = true;
  }

  void
  ToyProductFilterReplicated::respondToCloseInputFile(FileBlock const&,
                                                      Services const&)
  {
    respondToCloseInputFileCalled_ = true;
  }

  void
  ToyProductFilterReplicated::respondToOpenOutputFiles(FileBlock const&,
                                                       Services const&)
  {
    respondToOpenOutputFilesCalled_ = true;
  }

  void
  ToyProductFilterReplicated::respondToCloseOutputFiles(FileBlock const&,
                                                        Services const&)
  {
    respondToCloseOutputFilesCalled_ = true;
  }

  void
  ToyProductFilterReplicated::beginRun(Run const&, Services const&)
  {
    beginRunCalled_ = true;
  }

  void
  ToyProductFilterReplicated::beginSubRun(SubRun const&, Services const&)
  {
    beginSubRunCalled_ = true;
  }

  bool
  ToyProductFilterReplicated::filter(Event&, Services const&)
  {
    double val = 0.0;
    use_cpu_time(val);
    return true;
  }

  void
  ToyProductFilterReplicated::endSubRun(SubRun const&, Services const&)
  {
    endSubRunCalled_ = true;
  }

  void
  ToyProductFilterReplicated::endRun(Run const&, Services const&)
  {
    endRunCalled_ = true;
  }

  void
  ToyProductFilterReplicated::endJob(Services const&)
  {
    endJobCalled_ = true;
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::ToyProductFilterReplicated)
