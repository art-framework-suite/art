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
      Services const services{ScheduleID{}};
      beginJobWithServices(services);
    }

    void
    Analyzer::doEndJob()
    {
      Services const services{ScheduleID{}};
      endJobWithServices(services);
    }

    void
    Analyzer::doRespondToOpenInputFile(FileBlock const& fb)
    {
      Services const services{ScheduleID{}};
      respondToOpenInputFileWithServices(fb, services);
    }

    void
    Analyzer::doRespondToCloseInputFile(FileBlock const& fb)
    {
      Services const services{ScheduleID{}};
      respondToCloseInputFileWithServices(fb, services);
    }

    void
    Analyzer::doRespondToOpenOutputFiles(FileBlock const& fb)
    {
      Services const services{ScheduleID{}};
      respondToOpenOutputFilesWithServices(fb, services);
    }

    void
    Analyzer::doRespondToCloseOutputFiles(FileBlock const& fb)
    {
      Services const services{ScheduleID{}};
      respondToCloseOutputFilesWithServices(fb, services);
    }

    bool
    Analyzer::doBeginRun(RunPrincipal& rp, ModuleContext const& mc)
    {
      Run const r{rp, moduleDescription()};
      Services const services{mc.scheduleID()};
      beginRunWithServices(r, services);
      return true;
    }

    bool
    Analyzer::doEndRun(RunPrincipal& rp, ModuleContext const& mc)
    {
      Run const r{rp, moduleDescription()};
      Services const services{mc.scheduleID()};
      endRunWithServices(r, services);
      return true;
    }

    bool
    Analyzer::doBeginSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
    {
      SubRun const sr{srp, moduleDescription()};
      Services const services{mc.scheduleID()};
      beginSubRunWithServices(sr, services);
      return true;
    }

    bool
    Analyzer::doEndSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
    {
      SubRun const sr{srp, moduleDescription()};
      Services const services{mc.scheduleID()};
      endSubRunWithServices(sr, services);
      return true;
    }

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
        Services const services{mc.scheduleID()};
        analyzeWithServices(e, services);
        ++counts_passed;
      }
      return true;
    }

  } // namespace detail
} // namespace art
