#include "art/Framework/Core/ProcessingLimits.h"
#include "art/Framework/Core/detail/issue_reports.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Utilities/Exception.h"

namespace art {

  ProcessingLimits::ProcessingLimits(Config const& config)
    : remainingEvents_{config.maxEvents()}
    , remainingSubRuns_{config.maxSubRuns()}
    , reportFrequency_{config.reportFrequency()}
  {
    if (reportFrequency_ < 0) {
      throw Exception(errors::Configuration)
        << "reportFrequency has a negative value, which is not meaningful.";
    }
    std::string const runMode{"Runs"};
    std::string const runSubRunMode{"RunsAndSubRuns"};
    std::string const runSubRunEventMode{"RunsSubRunsAndEvents"};
    auto const& processingMode = config.processingMode();
    if (processingMode == runMode) {
      processingMode_ = InputSource::Runs;
    } else if (processingMode == runSubRunMode) {
      processingMode_ = InputSource::RunsAndSubRuns;
    } else if (processingMode != runSubRunEventMode) {
      throw Exception(errors::Configuration)
        << "The 'processingMode' parameter for sources has an illegal value '"
        << processingMode << "'\n"
        << "Legal values are '" << runSubRunEventMode << "', '" << runSubRunMode
        << "', or '" << runMode << "'.\n";
    }
  }

  bool
  ProcessingLimits::itemTypeAllowed(input::ItemType item_type) const noexcept
  {
    if ((item_type == input::IsEvent) &&
        (processingMode_ != InputSource::RunsSubRunsAndEvents)) {
      return false;
    }
    if ((item_type == input::IsSubRun) &&
        (processingMode_ == InputSource::Runs)) {
      return false;
    }
    return true;
  }

  bool
  ProcessingLimits::atLimit() const noexcept
  {
    return remainingEvents_ == 0 || remainingSubRuns_ == 0;
  }

  InputSource::ProcessingMode
  ProcessingLimits::processingMode() const noexcept
  {
    return processingMode_;
  }

  int
  ProcessingLimits::remainingEvents() const noexcept
  {
    return remainingEvents_;
  }

  int
  ProcessingLimits::remainingSubRuns() const noexcept
  {
    return remainingSubRuns_;
  }

  void
  ProcessingLimits::update(SubRunID const&)
  {
    if (remainingSubRuns_ > 0) {
      --remainingSubRuns_;
    }
  }

  void
  ProcessingLimits::update(EventID const& id)
  {
    if (remainingEvents_ > 0) {
      --remainingEvents_;
    }
    ++numberOfEventsRead_;
    if ((reportFrequency_ > 0) && !(numberOfEventsRead_ % reportFrequency_)) {
      detail::issue_reports(numberOfEventsRead_, id);
    }
  }
}
