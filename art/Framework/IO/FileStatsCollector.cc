#include "art/Framework/IO/FileStatsCollector.h"
// vim: set sw=2 expandtab :

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"

#include <set>
#include <string>
#include <vector>

using namespace std;

namespace art {

  FileStatsCollector::~FileStatsCollector() {}

  FileStatsCollector::FileStatsCollector(
    string const& moduleLabel,
    string const& processName,
    bool const enableLargeFileCatalogMetadata /*=true*/)
    : moduleLabel_{moduleLabel}
    , processName_{processName}
    , enableLargeFileCatalogMetadata_{enableLargeFileCatalogMetadata}
  {}

  string const&
  FileStatsCollector::moduleLabel() const
  {
    return moduleLabel_;
  }

  string const&
  FileStatsCollector::processName() const
  {
    return processName_;
  }

  boost::posix_time::ptime
  FileStatsCollector::outputFileOpenTime() const
  {
    return fo_;
  }

  boost::posix_time::ptime
  FileStatsCollector::outputFileCloseTime() const
  {
    return fc_;
  }

  SubRunID const&
  FileStatsCollector::lowestSubRunID() const
  {
    return lowestSubRun_;
  }

  SubRunID const&
  FileStatsCollector::highestSubRunID() const
  {
    return highestSubRun_;
  }

  EventID const&
  FileStatsCollector::lowestEventID() const
  {
    return lowestEventIDSeen_;
  }

  EventID const&
  FileStatsCollector::highestEventID() const
  {
    return highestEventIDSeen_;
  }

  string const&
  FileStatsCollector::lastOpenedInputFile() const
  {
    return lastOpenedInputFile_;
  }

  bool
  FileStatsCollector::fileCloseRecorded() const
  {
    return fileCloseRecorded_;
  }

  size_t
  FileStatsCollector::eventsThisFile() const
  {
    return nEvents_;
  }

  set<SubRunID> const&
  FileStatsCollector::seenSubRuns() const
  {
    return subRunsSeen_;
  }

  void
  FileStatsCollector::recordFileOpen()
  {
    resetStatistics_();
    if (enableLargeFileCatalogMetadata_ && !inputFilesSeen_.empty()) {
      inputFilesSeen_.emplace_back(lastOpenedInputFile_);
    }
    fo_ = boost::posix_time::second_clock::universal_time();
    fileCloseRecorded_ = false;
  }

  void
  FileStatsCollector::recordInputFile(string const& inputFileName)
  {
    if (enableLargeFileCatalogMetadata_ && !inputFileName.empty()) {
      inputFilesSeen_.emplace_back(inputFileName);
    }
    lastOpenedInputFile_ = inputFileName;
  }

  void
  FileStatsCollector::recordEvent(EventID const& id)
  {
    ++nEvents_;
    // Actually saw a real event that we've been asked to write, so
    // EventID should be valid.
    if (!lowestEventIDSeen_.isValid() || id < lowestEventIDSeen_) {
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
  FileStatsCollector::recordRun(RunID const& id)
  {
    if ((!lowestSubRun_.runID().isValid()) || id < lowestSubRun_.runID()) {
      lowestSubRun_ = SubRunID::invalidSubRun(id);
    }
    if (id > highestSubRun_.runID()) {
      // Sort-invalid-first gives the correct answer.
      highestSubRun_ = SubRunID::invalidSubRun(id);
    }
  }

  void
  FileStatsCollector::recordSubRun(SubRunID const& id)
  {
    recordRun(id.runID());
    if (id.runID() == lowestSubRun_.runID() &&
        (id.isValid() && ((!lowestSubRun_.isValid()) || // No valid subrun yet.
                          id < lowestSubRun_))) {
      lowestSubRun_ = id;
    }
    if (id > highestSubRun_) {
      // Sort-invalid-first gives the correct answer.
      highestSubRun_ = id;
    }
    if (enableLargeFileCatalogMetadata_) {
      subRunsSeen_.emplace(id);
    }
  }

  void
  FileStatsCollector::recordFileClose()
  {
    fc_ = boost::posix_time::second_clock::universal_time();
    fileCloseRecorded_ = true;
  }

  vector<string>
  FileStatsCollector::parents(bool want_basename) const
  {
    vector<string> result;
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
  FileStatsCollector::resetStatistics_()
  {
    fo_ = fc_ = boost::posix_time::ptime();
    lowestSubRun_ = highestSubRun_ = SubRunID{};
    inputFilesSeen_.clear();
    nEvents_ = 0ul;
    subRunsSeen_.clear();
  }

} // namespace art
