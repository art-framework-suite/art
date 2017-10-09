#include "art/Framework/IO/Root/detail/rangeSetFromFileIndex.h"

#include <algorithm>

using namespace art;

namespace {

  class next_run_in_file {
  public:
    bool
    operator()(FileIndex::Element const element) const
    {
      return element.getEntryType() == FileIndex::kRun;
    }
  };

  class next_subrun_in_run {
  public:
    next_subrun_in_run(RunID const runID) : runID_{runID} {}
    bool
    operator()(FileIndex::Element const element) const
    {
      return element.getEntryType() == FileIndex::kSubRun &&
             element.eventID_.runID() == runID_;
    }

  private:
    RunID const runID_;
  };

  class next_event_in_subrun {
  public:
    next_event_in_subrun(SubRunID const subRunID) : subRunID_{subRunID} {}
    bool
    operator()(FileIndex::Element const& element) const
    {
      return element.getEntryType() == FileIndex::kEvent &&
             element.eventID_.subRunID() == subRunID_;
    }

  private:
    SubRunID const subRunID_;
  };

  class end_of_subrun {
  public:
    end_of_subrun(SubRunID const subRunID) : subRunID_{subRunID} {}
    bool
    operator()(FileIndex::Element const element) const
    {
      return element.eventID_.subRunID() != subRunID_;
    }

  private:
    SubRunID const subRunID_;
  };

  class end_of_run {
  public:
    end_of_run(RunID const runID) : runID_{runID} {}
    bool
    operator()(FileIndex::Element const element) const
    {
      return element.eventID_.runID() != runID_;
    }

  private:
    RunID const runID_;
  };
}

RangeSet
detail::rangeSetFromFileIndex(FileIndex const& fileIndex,
                              SubRunID const subRunID,
                              bool const compactRanges)
{
  RangeSet rangeSet{subRunID.run()};
  auto begin = std::cbegin(fileIndex);
  auto end = std::cend(fileIndex);

  auto subrun_begin =
    std::find_if(begin, end, next_subrun_in_run{subRunID.runID()});
  if (subrun_begin == end) {
    // SubRun does not exist in the provided fileIndex
    return rangeSet;
  }

  auto event_it =
    std::find_if(subrun_begin, end, next_event_in_subrun{subRunID});
  if (event_it == end) {
    // SubRun has no event entries entries
    return rangeSet;
  }

  auto const& eid = event_it->eventID_;
  auto event_end =
    std::find_if_not(event_it, end, next_event_in_subrun{subRunID});

  if (compactRanges) {
    auto const count = std::distance(event_it, event_end);
    if (count < 1) {
      // Should never get here, and should probably throw
      return rangeSet;
    }

    auto const subrun = subRunID.subRun();
    auto const ebegin = eid.event();
    auto const eend = (count == 1) ?
                        eid.next().event() :
                        std::prev(event_end)->eventID_.next().event();
    rangeSet.emplace_range(subrun, ebegin, eend);
    return rangeSet;
  }

  std::for_each(event_it, event_end, [&rangeSet](auto const& element) {
    rangeSet.update(element.eventID_);
  });
  return rangeSet;
}

RangeSet
detail::rangeSetFromFileIndex(FileIndex const& fileIndex,
                              RunID const runID,
                              bool const compactRanges)
{
  auto const run = runID.run();
  auto const begin = std::cbegin(fileIndex);
  auto const end = std::cend(fileIndex);

  RangeSet rangeSet{run};

  // Loop through SubRuns in Run
  auto subrun_begin = std::find_if(begin, end, next_subrun_in_run{runID});
  if (subrun_begin == end) {
    // Run has no entries -- early bail out
    return rangeSet;
  }

  auto const run_end = std::find_if(subrun_begin, end, end_of_run{runID});
  while (subrun_begin != run_end) {
    auto const subRunID = subrun_begin->eventID_.subRunID();
    auto rs = rangeSetFromFileIndex(fileIndex, subRunID, compactRanges);
    if (rs.is_valid()) {
      rangeSet.merge(rs);
    }
    auto subrun_end = std::find_if(subrun_begin, end, end_of_subrun{subRunID});
    subrun_begin = subrun_end;
  }

  return rangeSet;
}
