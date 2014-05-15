#include "art/Framework/IO/FileStatsCollector.h"
#include "boost/date_time/posix_time/posix_time.hpp"

#include <string>

art::FileStatsCollector::
FileStatsCollector(std::string const & moduleLabel,
                     std::string const & processName)
  :
  moduleLabel_(moduleLabel),
  processName_(processName),
  lowest_(),
  highest_(),
  fo_(),
  fc_(),
  seqNo_(0ul),
  lastOpenedInputFile_()
{
}

void
art::FileStatsCollector::
recordFileOpen()
{
  reset_(); // Reset statistics.
  fo_ = boost::posix_time::second_clock::universal_time();
}

void
art::FileStatsCollector::
recordEvent(EventID const & id)
{
  // Don't care about the event number at the moment.
  recordSubRun(id.subRunID());
}

void
art::FileStatsCollector::
recordRun(RunID const & id)
{
  if ((!lowest_.runID().isValid()) ||
      id < lowest_.runID()) {
    lowest_ = SubRunID::invalidSubRun(id);
  }
  if ((!highest_.runID().isValid()) ||
      id > highest_.runID()) {
    highest_ = SubRunID::invalidSubRun(id);
  }
}

void
art::FileStatsCollector::
recordInputFile(std::string const & inputFileName)
{
  lastOpenedInputFile_ = inputFileName;
}

void
art::FileStatsCollector::
recordSubRun(SubRunID const & id)
{
  if ((!lowest_.runID().isValid()) || // No lowest run yet.
      id.runID() < lowest_.runID() || // New lower run.
      (id.runID() == lowest_.runID() &&
       (id.isValid() &&
        ((!lowest_.isValid()) || // No valid subrun yet.
         id < lowest_)))) {
    lowest_ = id;
  }
  if (id > highest_) { // Sort-invalid-first gives the correct answer.
    highest_ = id;
  }
}

void
art::FileStatsCollector::
recordFileClose()
{
  fc_ =  boost::posix_time::second_clock::universal_time();
  ++seqNo_;
}

void
art::FileStatsCollector::
reset_()
{
  fo_ =
    fc_ = boost::posix_time::ptime();
  lowest_ = SubRunID();
  highest_ = SubRunID();
}
