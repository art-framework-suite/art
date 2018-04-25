#include "art/Framework/Core/EDAnalyzer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/EventObserverBase.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>

using namespace hep::concurrency;
using namespace std;

namespace art {
  namespace detail {

    Analyzer::~Analyzer() noexcept = default;

    Analyzer::Analyzer(fhicl::ParameterSet const& pset)
      : EventObserverBase{pset}
    {}

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
    Analyzer::doBeginRun(RunPrincipal& rp,
                         cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      Run const r{rp, md_};
      beginRun(r);
      return true;
    }

    void
    Analyzer::beginRun(Run const&)
    {}

    bool
    Analyzer::doEndRun(RunPrincipal& rp,
                       cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      Run const r{rp, md_};
      endRun(r);
      return true;
    }

    void
    Analyzer::endRun(Run const&)
    {}

    bool
    Analyzer::doBeginSubRun(SubRunPrincipal& srp,
                            cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      SubRun const sr{srp, md_};
      beginSubRun(sr);
      return true;
    }

    void
    Analyzer::beginSubRun(SubRun const&)
    {}

    bool
    Analyzer::doEndSubRun(SubRunPrincipal& srp,
                          cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      SubRun const sr{srp, md_};
      endSubRun(sr);
      return true;
    }

    void
    Analyzer::endSubRun(SubRun const&)
    {}

    bool
    Analyzer::doEvent(EventPrincipal& ep,
                      ScheduleID const /*si*/,
                      CurrentProcessingContext const* cpc,
                      std::atomic<std::size_t>& counts_run,
                      std::atomic<std::size_t>& counts_passed,
                      std::atomic<std::size_t>& /*counts_failed*/)
    {
      detail::CPCSentry sentry{*cpc};
      detail::PVSentry pvSentry{processAndEventSelectors()};
      Event const e{ep, md_};
      if (wantAllEvents() || wantEvent(e)) {
        ++counts_run;
        analyze(e);
        ++counts_passed;
      }
      return true;
    }

  } // namespace detail
} // namespace art
