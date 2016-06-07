#ifndef art_Framework_IO_FileStatsCollector_h
#define art_Framework_IO_FileStatsCollector_h

#include "canvas/Persistency/Provenance/EventID.h"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "boost/regex.hpp"

#include <cstddef> // For size_t.
#include <string>

namespace art {
  class FileStatsCollector;
}

class art::FileStatsCollector {
public:
  FileStatsCollector(std::string const & moduleLabel,
                     std::string const & processName);

  void recordFileOpen();
  void recordInputFile(std::string const & inputFileName);
  void recordEvent(EventID const & id);
  void recordRun(RunID const & id);
  void recordSubRun(SubRunID const & id);
  void recordFileClose();

  std::string const & moduleLabel() const;
  std::string const & processName() const;
  boost::posix_time::ptime outputFileOpenTime() const;
  boost::posix_time::ptime outputFileCloseTime() const;
  SubRunID const & lowestSubRunID() const;
  SubRunID const & highestSubRunID() const;
  EventID const & lowestEventID() const;
  EventID const & highestEventID() const;
  std::string const & lastOpenedInputFile() const;
  std::vector<std::string> parents(bool want_basename = true) const;
  size_t sequenceNum() const;
  size_t eventsThisFile() const;
  std::set<SubRunID> const & seenSubRuns() const;

private:
  void reset_(); // Reset statistics without renaming.

  std::string const moduleLabel_;
  std::string const processName_;
  SubRunID lowestSubRun_ {};
  SubRunID highestSubRun_ {};
  EventID lowestEventIDSeen_ {};
  EventID highestEventIDSeen_ {};
  boost::posix_time::ptime fo_ {};
  boost::posix_time::ptime fc_ {};
  size_t seqNo_ {};
  std::string lastOpenedInputFile_ {};
  std::vector<std::string> inputFilesSeen_ {};
  size_t nEvents_ {};
  std::set<SubRunID> subRunsSeen_ {};
};

inline
std::string const &
art::FileStatsCollector::
moduleLabel() const
{
  return moduleLabel_;
}

inline
std::string const &
art::FileStatsCollector::
processName() const
{
  return processName_;
}

inline
boost::posix_time::ptime
art::FileStatsCollector::
outputFileOpenTime() const
{
  return fo_;
}

inline
boost::posix_time::ptime
art::FileStatsCollector::
outputFileCloseTime() const
{
  return fc_;
}

inline
art::SubRunID const &
art::FileStatsCollector::
lowestSubRunID() const
{
  return lowestSubRun_;
}

inline
art::SubRunID const &
art::FileStatsCollector::
highestSubRunID() const
{
  return highestSubRun_;
}

inline
art::EventID const &
art::FileStatsCollector::
lowestEventID() const
{
  return lowestEventIDSeen_;
}

inline
art::EventID const &
art::FileStatsCollector::
highestEventID() const
{
  return highestEventIDSeen_;
}

inline
std::string const &
art::FileStatsCollector::
lastOpenedInputFile() const
{
  return lastOpenedInputFile_;
}

inline
size_t
art::FileStatsCollector::
sequenceNum() const
{
  return seqNo_;
}

inline
size_t
art::FileStatsCollector::
eventsThisFile() const
{
  return nEvents_;
}

inline
std::set<art::SubRunID> const &
art::FileStatsCollector::
seenSubRuns() const
{
  return subRunsSeen_;
}

#endif /* art_Framework_IO_FileStatsCollector_h */

// Local Variables:
// mode: c++
// End:
