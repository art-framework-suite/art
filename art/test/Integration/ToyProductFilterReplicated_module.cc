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
    explicit ToyProductFilterReplicated(Parameters const& p,
                                        ProcessingFrame const&);
    ~ToyProductFilterReplicated();

  private:
    void beginJob(ProcessingFrame const&) override;
    void respondToOpenInputFile(FileBlock const&,
                                ProcessingFrame const&) override;
    void respondToCloseInputFile(FileBlock const&,
                                 ProcessingFrame const&) override;
    void respondToOpenOutputFiles(FileBlock const&,
                                  ProcessingFrame const&) override;
    void respondToCloseOutputFiles(FileBlock const&,
                                   ProcessingFrame const&) override;
    void beginRun(Run const&, ProcessingFrame const&) override;
    void beginSubRun(SubRun const&, ProcessingFrame const&) override;
    bool filter(Event&, ProcessingFrame const&) override;
    void endSubRun(SubRun const&, ProcessingFrame const&) override;
    void endRun(Run const&, ProcessingFrame const&) override;
    void endJob(ProcessingFrame const&) override;

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
    ProcessingFrame const& frame)
    : ReplicatedFilter{p, frame}
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
  ToyProductFilterReplicated::beginJob(ProcessingFrame const&)
  {
    beginJobCalled_ = true;
  }

  void
  ToyProductFilterReplicated::respondToOpenInputFile(FileBlock const&,
                                                     ProcessingFrame const&)
  {
    respondToOpenInputFileCalled_ = true;
  }

  void
  ToyProductFilterReplicated::respondToCloseInputFile(FileBlock const&,
                                                      ProcessingFrame const&)
  {
    respondToCloseInputFileCalled_ = true;
  }

  void
  ToyProductFilterReplicated::respondToOpenOutputFiles(FileBlock const&,
                                                       ProcessingFrame const&)
  {
    respondToOpenOutputFilesCalled_ = true;
  }

  void
  ToyProductFilterReplicated::respondToCloseOutputFiles(FileBlock const&,
                                                        ProcessingFrame const&)
  {
    respondToCloseOutputFilesCalled_ = true;
  }

  void
  ToyProductFilterReplicated::beginRun(Run const&, ProcessingFrame const&)
  {
    beginRunCalled_ = true;
  }

  void
  ToyProductFilterReplicated::beginSubRun(SubRun const&, ProcessingFrame const&)
  {
    beginSubRunCalled_ = true;
  }

  bool
  ToyProductFilterReplicated::filter(Event&, ProcessingFrame const&)
  {
    double val = 0.0;
    use_cpu_time(val);
    return true;
  }

  void
  ToyProductFilterReplicated::endSubRun(SubRun const&, ProcessingFrame const&)
  {
    endSubRunCalled_ = true;
  }

  void
  ToyProductFilterReplicated::endRun(Run const&, ProcessingFrame const&)
  {
    endRunCalled_ = true;
  }

  void
  ToyProductFilterReplicated::endJob(ProcessingFrame const&)
  {
    endJobCalled_ = true;
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::ToyProductFilterReplicated)
