#include "art/Framework/IO/FileStatsCollector.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"

#include <string>

using boost::posix_time::ptime;
namespace {
  auto now = boost::posix_time::second_clock::universal_time;
}

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
  fo_ = now();
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
art::FileStatsCollector::recordRun(RunID const& id)
{
  if (!lowestRun_.isValid()) {
    lowestRun_ = highestRun_ = id;
    lowestRunStartTime_ = highestRunStartTime_ = now();
    return;
  }

  if (id < lowestRun_) {
    lowestRun_ = id;
    lowestRunStartTime_ = now();
    if (lowestSubRun_.runID() != lowestRun_) {
      lowestSubRun_ = SubRunID{};
      lowestSubRunStartTime_ = ptime{};
    }
  } else if (id > highestRun_) {
    highestRun_ = id;
    highestRunStartTime_ = now();
    if (highestSubRun_.runID() != highestRun_) {
      highestSubRun_ = SubRunID{};
      highestSubRunStartTime_ = ptime{};
    }
  }
}

void
art::FileStatsCollector::recordSubRun(SubRunID const& id)
{
  if (!lowestSubRun_.isValid()) {
    lowestSubRun_ = highestSubRun_ = id;
    lowestSubRunStartTime_ = highestSubRunStartTime_ = now();
  } else if (id < lowestSubRun_) {
    lowestSubRun_ = id;
    lowestSubRunStartTime_ = now();
  } else if (id > highestSubRun_) {
    highestSubRun_ = id;
    highestSubRunStartTime_ = now();
  }
  subRunsSeen_.emplace(id);
}

void
art::FileStatsCollector::recordFileClose()
{
  fc_ = now();
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
  fo_ = fc_ = ptime{};
  lowestRun_ = highestRun_ = RunID{};
  lowestSubRun_ = highestSubRun_ = SubRunID{};
  lowestRunStartTime_ = highestRunStartTime_ = ptime{};
  lowestSubRunStartTime_ = highestSubRunStartTime_ = ptime{};
  inputFilesSeen_.clear();
  nEvents_ = 0ul;
  subRunsSeen_.clear();
}
