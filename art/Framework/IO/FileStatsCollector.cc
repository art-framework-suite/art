#include "art/Framework/IO/FileStatsCollector.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"

#include <string>

art::FileStatsCollector::FileStatsCollector(std::string const& moduleLabel,
                                            std::string const& processName)
  : moduleLabel_{moduleLabel}, processName_{processName}
{}

void
art::FileStatsCollector::recordFileOpen()
{
  resetStatistics_();
  if (!inputFilesSeen_.empty()) {
    inputFilesSeen_.emplace_back(lastOpenedInputFile_);
  }
  fo_ = boost::posix_time::second_clock::universal_time();
  fileCloseRecorded_ = false;
}

void
art::FileStatsCollector::recordInputFile(std::string const& inputFileName)
{
  if (!inputFileName.empty()) {
    inputFilesSeen_.emplace_back(inputFileName);
  }
  lastOpenedInputFile_ = inputFileName;
}

void
art::FileStatsCollector::recordEvent(EventID const& id)
{
  ++nEvents_;
  if (!lowestEventIDSeen_.isValid()) {
    lowestEventIDSeen_ = highestEventIDSeen_ = id;
  } else if (id < lowestEventIDSeen_) {
    lowestEventIDSeen_ = id;
  } else if (id > highestEventIDSeen_) {
    highestEventIDSeen_ = id;
  }
}

void
art::FileStatsCollector::recordRun(RunID const& id, Timestamp const ts)
{
  if (!lowestRun_.isValid()) {
    lowestRun_ = highestRun_ = id;
    lowestRunStartTime_ = highestRunStartTime_ = ts;
    return;
  }

  if (id < lowestRun_) {
    lowestRun_ = id;
    lowestRunStartTime_ = ts;
    if (lowestSubRun_.runID() != lowestRun_) {
      lowestSubRun_ = SubRunID{};
      lowestSubRunStartTime_ = Timestamp::invalidTimestamp();
    }
  } else if (id > highestRun_) {
    highestRun_ = id;
    highestRunStartTime_ = ts;
    if (highestSubRun_.runID() != highestRun_) {
      highestSubRun_ = SubRunID{};
      highestSubRunStartTime_ = Timestamp::invalidTimestamp();
    }
  }
}

void
art::FileStatsCollector::recordSubRun(SubRunID const& id, Timestamp const ts)
{
  if (!lowestSubRun_.isValid()) {
    lowestSubRun_ = highestSubRun_ = id;
    lowestSubRunStartTime_ = highestSubRunStartTime_ = ts;
  } else if (id < lowestSubRun_) {
    lowestSubRun_ = id;
    lowestSubRunStartTime_ = ts;
  } else if (id > highestSubRun_) {
    highestSubRun_ = id;
    highestSubRunStartTime_ = ts;
  }
  subRunsSeen_.emplace(id);
}

void
art::FileStatsCollector::recordFileClose()
{
  fc_ = boost::posix_time::second_clock::universal_time();
  fileCloseRecorded_ = true;
}

std::vector<std::string>
art::FileStatsCollector::parents(bool const want_basename) const
{
  std::vector<std::string> result;
  if (want_basename) {
    result.reserve(inputFilesSeen_.size());
    for (auto const& ifile : inputFilesSeen_) {
      boost::filesystem::path const ifp{ifile};
      result.emplace_back(ifp.filename().native());
    }
  } else {
    result = inputFilesSeen_;
  }
  return result;
}

void
art::FileStatsCollector::resetStatistics_()
{
  fo_ = fc_ = boost::posix_time::ptime{};
  lowestRun_ = highestRun_ = RunID{};
  lowestSubRun_ = highestSubRun_ = SubRunID{};
  inputFilesSeen_.clear();
  nEvents_ = 0ul;
  subRunsSeen_.clear();
}
