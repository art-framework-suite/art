#ifndef art_Framework_IO_FileStatsCollector_h
#define art_Framework_IO_FileStatsCollector_h

//===============================================================
// The FileStatsCollector stores only the information required to
// assemble the final name of the file.  It does not assemble the
// filename itself--that is the role of the PostCloseFileRenamer.
//===============================================================

#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "canvas/Persistency/Provenance/EventID.h"

#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace art {

  class FileStatsCollector {
    // Special Member Functions
  public:
    ~FileStatsCollector();
    FileStatsCollector(std::string const& moduleLabel,
                       std::string const& processName,
                       bool const enableLargeFileCatalogMetadata = true);

    // API
  public:
    void recordFileOpen();
    void recordInputFile(std::string const& inputFileName);
    void recordEvent(EventID const& id);
    void recordRun(RunID const& id);
    void recordSubRun(SubRunID const& id);
    void recordFileClose();
    std::string const& moduleLabel() const;
    std::string const& processName() const;
    boost::posix_time::ptime outputFileOpenTime() const;
    boost::posix_time::ptime outputFileCloseTime() const;
    SubRunID const& lowestSubRunID() const;
    SubRunID const& highestSubRunID() const;
    EventID const& lowestEventID() const;
    EventID const& highestEventID() const;
    std::string const& lastOpenedInputFile() const;
    std::vector<std::string> parents(bool want_basename = true) const;
    bool fileCloseRecorded() const;
    std::size_t eventsThisFile() const;
    std::set<SubRunID> const& seenSubRuns() const;

    // Implementation Details
  private:
    void resetStatistics_(); // Does not rename.

    // Member Data
  private:
    std::string const moduleLabel_;
    std::string const processName_;
    SubRunID lowestSubRun_{};
    SubRunID highestSubRun_{};
    EventID lowestEventIDSeen_{};
    EventID highestEventIDSeen_{};
    boost::posix_time::ptime fo_{};
    boost::posix_time::ptime fc_{};
    bool fileCloseRecorded_{false};
    std::string lastOpenedInputFile_{};
    std::vector<std::string> inputFilesSeen_{};
    std::size_t nEvents_{};
    std::set<SubRunID> subRunsSeen_{};
    bool enableLargeFileCatalogMetadata_{true};
  };

} // namespace art

#endif /* art_Framework_IO_FileStatsCollector_h */

// Local Variables:
// mode: c++
// End:
