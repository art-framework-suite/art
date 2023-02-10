#include "art/Framework/Core/ProcessingLimits.h"
#include "art/Framework/Core/detail/issue_reports.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Utilities/Exception.h"

namespace art {

  ProcessingLimits::ProcessingLimits(
    Config const& config,
    std::function<input::ItemType()> nextItemType)
    : processingMode_{InputSource::mode(config.processingMode())}
    , remainingEvents_{config.maxEvents()}
    , remainingSubRuns_{config.maxSubRuns()}
    , reportFrequency_{config.reportFrequency()}
    , nextItemType_{std::move(nextItemType)}
  {
    if (reportFrequency_ < 0) {
      throw Exception(errors::Configuration)
        << "reportFrequency has a negative value, which is not meaningful.";
    }
  }

  input::ItemType
  ProcessingLimits::nextItemType()
  {
    if (remainingEvents_ == 0 || remainingSubRuns_ == 0) {
      return input::IsStop;
    }

    auto item_type = nextItemType_();
    while (not allowed_(item_type)) {
      item_type = nextItemType_();
    }
    return item_type;
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

  bool
  ProcessingLimits::allowed_(input::ItemType item_type) const noexcept
  {
    if (processingMode_ == InputSource::RunsSubRunsAndEvents) {
      return true;
    }

    // Events no longer allowed
    if (item_type == input::IsEvent) {
      return false;
    }
    if (processingMode_ == InputSource::RunsAndSubRuns) {
      return true;
    }

    // SubRuns no longer allowed
    return item_type != input::IsSubRun;
  }
}
