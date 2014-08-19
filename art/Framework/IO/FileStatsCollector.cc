#include "art/Framework/IO/FileStatsCollector.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"

#include <string>

art::FileStatsCollector::
FileStatsCollector(std::string const & moduleLabel,
                     std::string const & processName)
  :
  moduleLabel_(moduleLabel),
  processName_(processName),
  lowestSubRun_(),
  highestSubRun_(),
  lowestEventIDSeen_(),
  highestEventIDSeen_(),
  fo_(),
  fc_(),
  seqNo_(0ul),
  lastOpenedInputFile_(),
  inputFilesSeen_(),
  nEvents_(0ul),
  subRunsSeen_()
{
}

void
art::FileStatsCollector::
recordFileOpen()
{
  reset_(); // Reset statistics.
  if (!inputFilesSeen_.empty()) {
    inputFilesSeen_.emplace_back(lastOpenedInputFile_);
  }
  fo_ = boost::posix_time::second_clock::universal_time();
}

void
art::FileStatsCollector::
recordInputFile(std::string const & inputFileName)
{
  if (!inputFileName.empty()) {
    inputFilesSeen_.emplace_back(inputFileName);
  }
  lastOpenedInputFile_ = inputFileName;
}

void
art::FileStatsCollector::
recordEvent(EventID const & id)
{
  ++nEvents_;
  // Actually saw a real event that we've been asked to write, so
  // EventID should be valid.
  if (!lowestEventIDSeen_.isValid() ||
      id < lowestEventIDSeen_) {
    lowestEventIDSeen_ = id;
  }
  if (id > highestEventIDSeen_) {
    // Sort-invalid-first gives the correct answer.
    highestEventIDSeen_ = id;
  }
  // Record that we have seen this SubRunID too.
  recordSubRun(id.subRunID());
}

void
art::FileStatsCollector::
recordRun(RunID const & id)
{
  if ((!lowestSubRun_.runID().isValid()) ||
      id < lowestSubRun_.runID()) {
    lowestSubRun_ = SubRunID::invalidSubRun(id);
  }
  if (id > highestSubRun_.runID()) {
    // Sort-invalid-first gives the correct answer.
    highestSubRun_ = SubRunID::invalidSubRun(id);
  }
}

void
art::FileStatsCollector::
recordSubRun(SubRunID const & id)
{
  recordRun(id.runID());
  if (id.runID() == lowestSubRun_.runID() &&
      (id.isValid() &&
       ((!lowestSubRun_.isValid()) || // No valid subrun yet.
        id < lowestSubRun_))) {
    lowestSubRun_ = id;
  }
  if (id > highestSubRun_) {
    // Sort-invalid-first gives the correct answer.
    highestSubRun_ = id;
  }
  subRunsSeen_.emplace(id);
}

void
art::FileStatsCollector::
recordFileClose()
{
  fc_ =  boost::posix_time::second_clock::universal_time();
  ++seqNo_;
}

std::vector<std::string>
art::FileStatsCollector::
parents(bool want_basename) const
{
  std::vector<std::string> result;
  if (want_basename) {
    result.reserve(inputFilesSeen_.size());
    for (auto const & ifile : inputFilesSeen_) {
      boost::filesystem::path const ifp(ifile);
      result.emplace_back(ifp.filename().native());
    }
  } else {
    result = inputFilesSeen_;
  }
  return result;
}



void
art::FileStatsCollector::
reset_()
{
  fo_ =
    fc_ = boost::posix_time::ptime();
  lowestSubRun_ = SubRunID();
  highestSubRun_ = SubRunID();
  inputFilesSeen_.clear();
  nEvents_ = 0ul;
  subRunsSeen_.clear();
}
