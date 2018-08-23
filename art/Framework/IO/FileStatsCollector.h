#ifndef art_Framework_IO_FileStatsCollector_h
#define art_Framework_IO_FileStatsCollector_h

//===============================================================
// The FileStatsCollector stores only the information required to
// assemble the final name of the file.  It does not assemble the
// filename itself--that is the role of the PostCloseFileRenamer.
//===============================================================

#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"

#include <cstddef> // For std::size_t.
#include <set>
#include <string>
#include <vector>

namespace art {
  class FileStatsCollector;
}

class art::FileStatsCollector {
public:
  FileStatsCollector(std::string const& moduleLabel,
                     std::string const& processName);

  void recordFileOpen();
  void recordInputFile(std::string const& inputFileName);
  void recordEvent(EventID const& id);
  void recordRun(RunID const& id,
                 Timestamp const startTime = Timestamp::invalidTimestamp());
  void recordSubRun(SubRunID const& id,
                    Timestamp const startTime = Timestamp::invalidTimestamp());
  void recordFileClose();

  std::string const& moduleLabel() const;
  std::string const& processName() const;
  boost::posix_time::ptime outputFileOpenTime() const;
  boost::posix_time::ptime outputFileCloseTime() const;
  Timestamp lowestRunStartTime() const;
  Timestamp highestRunStartTime() const;
  Timestamp lowestSubRunStartTime() const;
  Timestamp highestSubRunStartTime() const;
  RunID lowestRunID() const;
  RunID highestRunID() const;
  SubRunID const& lowestSubRunID() const;
  SubRunID const& highestSubRunID() const;
  EventID const& lowestEventID() const;
  EventID const& highestEventID() const;
  std::string const& lastOpenedInputFile() const;
  std::vector<std::string> parents(bool want_basename = true) const;
  bool fileCloseRecorded() const;
  std::size_t eventsThisFile() const;
  std::set<SubRunID> const& seenSubRuns() const;

private:
  void resetStatistics_(); // Does not rename.

  std::string const moduleLabel_;
  std::string const processName_;
  RunID lowestRun_{};
  RunID highestRun_{};
  SubRunID lowestSubRun_{};
  SubRunID highestSubRun_{};
  EventID lowestEventIDSeen_{};
  EventID highestEventIDSeen_{};
  boost::posix_time::ptime fo_{};
  boost::posix_time::ptime fc_{};
  Timestamp lowestRunStartTime_{};
  Timestamp highestRunStartTime_{};
  Timestamp lowestSubRunStartTime_{};
  Timestamp highestSubRunStartTime_{};
  bool fileCloseRecorded_{false};
  std::string lastOpenedInputFile_{};
  std::vector<std::string> inputFilesSeen_{};
  std::size_t nEvents_{};
  std::set<SubRunID> subRunsSeen_{};
};

inline std::string const&
art::FileStatsCollector::moduleLabel() const
{
  return moduleLabel_;
}

inline std::string const&
art::FileStatsCollector::processName() const
{
  return processName_;
}

inline boost::posix_time::ptime
art::FileStatsCollector::outputFileOpenTime() const
{
  return fo_;
}

inline boost::posix_time::ptime
art::FileStatsCollector::outputFileCloseTime() const
{
  return fc_;
}

inline art::Timestamp
art::FileStatsCollector::lowestRunStartTime() const
{
  return lowestRunStartTime_;
}

inline art::Timestamp
art::FileStatsCollector::highestRunStartTime() const
{
  return highestRunStartTime_;
}

inline art::Timestamp
art::FileStatsCollector::lowestSubRunStartTime() const
{
  return lowestSubRunStartTime_;
}

inline art::Timestamp
art::FileStatsCollector::highestSubRunStartTime() const
{
  return highestSubRunStartTime_;
}

inline art::RunID
art::FileStatsCollector::lowestRunID() const
{
  return lowestRun_;
}

inline art::RunID
art::FileStatsCollector::highestRunID() const
{
  return highestRun_;
}

inline art::SubRunID const&
art::FileStatsCollector::lowestSubRunID() const
{
  return lowestSubRun_;
}

inline art::SubRunID const&
art::FileStatsCollector::highestSubRunID() const
{
  return highestSubRun_;
}

inline art::EventID const&
art::FileStatsCollector::lowestEventID() const
{
  return lowestEventIDSeen_;
}

inline art::EventID const&
art::FileStatsCollector::highestEventID() const
{
  return highestEventIDSeen_;
}

inline std::string const&
art::FileStatsCollector::lastOpenedInputFile() const
{
  return lastOpenedInputFile_;
}

inline bool
art::FileStatsCollector::fileCloseRecorded() const
{
  return fileCloseRecorded_;
}

inline std::size_t
art::FileStatsCollector::eventsThisFile() const
{
  return nEvents_;
}

inline std::set<art::SubRunID> const&
art::FileStatsCollector::seenSubRuns() const
{
  return subRunsSeen_;
}

#endif /* art_Framework_IO_FileStatsCollector_h */

// Local Variables:
// mode: c++
// End:
