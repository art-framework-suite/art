#ifndef art_Framework_IO_PostCloseFileRenamer_h
#define art_Framework_IO_PostCloseFileRenamer_h

#include "art/Persistency/Provenance/EventID.h"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "boost/regex.hpp"

#include <string>
extern "C" {
#include <sys/time.h>
}
#include <utility>

namespace art {
  class PostCloseFileRenamer;
}

class art::PostCloseFileRenamer {
public:
  PostCloseFileRenamer(std::string const & filePattern,
                       std::string const & moduleLabel,
                       std::string const & processName);

  std::string parentPath() const;

  void recordFileOpen();
  void recordEvent(EventID const & id);
  void recordInputFile(std::string const & inputFileName);
  void recordRun(RunID const & id);
  void recordSubRun(SubRunID const & id);
  void recordFileClose();

  std::string applySubstitutions() const;

  void maybeRenameFile(std::string const & inPath); // Rename given file.

private:
  std::string subInputFileName_(boost::smatch const & match) const;
  std::string subTimestamp_(boost::smatch const & match) const;
  std::string subFilledNumeric_(boost::smatch const & match) const;
  void reset_(); // Reset statistics without renaming.

  std::string filePattern_;
  std::string moduleLabel_;
  std::string processName_;
  SubRunID lowest_;
  SubRunID highest_;
  boost::posix_time::ptime fo_;
  boost::posix_time::ptime fc_;
  size_t seqNo_;
  std::string lastOpenedInputFile_;
};

#endif /* art_Framework_IO_PostCloseFileRenamer_h */

// Local Variables:
// mode: c++
// End:
