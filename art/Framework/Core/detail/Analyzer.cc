#include "art/Framework/Core/EDAnalyzer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/ModuleContext.h"

#include <ostream>

using namespace hep::concurrency;
using namespace std;

namespace art {
  namespace detail {

    Analyzer::~Analyzer() noexcept = default;

    Analyzer::Analyzer(fhicl::ParameterSet const& pset) : Observer{pset} {}

    void
    Analyzer::doBeginJob()
    {
      setupQueues();
      beginJob();
    }

    void
    Analyzer::beginJob()
    {}

    void
    Analyzer::doEndJob()
    {
      endJob();
    }

    void
    Analyzer::endJob()
    {}

    void
    Analyzer::doRespondToOpenInputFile(FileBlock const& fb)
    {
      respondToOpenInputFile(fb);
    }

    void
    Analyzer::respondToOpenInputFile(FileBlock const&)
    {}

    void
    Analyzer::doRespondToCloseInputFile(FileBlock const& fb)
    {
      respondToCloseInputFile(fb);
    }

    void
    Analyzer::respondToCloseInputFile(FileBlock const&)
    {}

    void
    Analyzer::doRespondToOpenOutputFiles(FileBlock const& fb)
    {
      respondToOpenOutputFiles(fb);
    }

    void
    Analyzer::respondToOpenOutputFiles(FileBlock const&)
    {}

    void
    Analyzer::doRespondToCloseOutputFiles(FileBlock const& fb)
    {
      respondToCloseOutputFiles(fb);
    }

    void
    Analyzer::respondToCloseOutputFiles(FileBlock const&)
    {}

    bool
    Analyzer::doBeginRun(RunPrincipal& rp, ModuleContext const&)
    {
      Run const r{rp, moduleDescription()};
      beginRun(r);
      return true;
    }

    void
    Analyzer::beginRun(Run const&)
    {}

    bool
    Analyzer::doEndRun(RunPrincipal& rp, ModuleContext const&)
    {
      Run const r{rp, moduleDescription()};
      endRun(r);
      return true;
    }

    void
    Analyzer::endRun(Run const&)
    {}

    bool
    Analyzer::doBeginSubRun(SubRunPrincipal& srp, ModuleContext const&)
    {
      SubRun const sr{srp, moduleDescription()};
      beginSubRun(sr);
      return true;
    }

    void
    Analyzer::beginSubRun(SubRun const&)
    {}

    bool
    Analyzer::doEndSubRun(SubRunPrincipal& srp, ModuleContext const&)
    {
      SubRun const sr{srp, moduleDescription()};
      endSubRun(sr);
      return true;
    }

    void
    Analyzer::endSubRun(SubRun const&)
    {}

    bool
    Analyzer::doEvent(EventPrincipal& ep,
                      ModuleContext const& mc,
                      std::atomic<std::size_t>& counts_run,
                      std::atomic<std::size_t>& counts_passed,
                      std::atomic<std::size_t>& /*counts_failed*/)
    {
      detail::PVSentry pvSentry{processAndEventSelectors()};
      Event const e{ep, moduleDescription()};
      if (wantAllEvents() || wantEvent(e)) {
        ++counts_run;
        analyzeWithScheduleID(e, mc.scheduleID());
        ++counts_passed;
      }
      return true;
    }

  } // namespace detail
} // namespace art