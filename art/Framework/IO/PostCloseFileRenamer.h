#ifndef art_Framework_IO_PostCloseFileRenamer_h
#define art_Framework_IO_PostCloseFileRenamer_h

#include "art/Persistency/Provenance/EventID.h"
#include "boost/date_time/posix_time/posix_time_types.hpp"

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
  void recordFileClose();

  std::string applySubstitutions() const;

  void maybeRenameFile(std::string const & inPath); // Rename given file.

private:
  void reset_(); // Reset statistics without renaming.

  std::string filePattern_;
  std::string moduleLabel_;
  std::string processName_;
  EventID lowest_;
  EventID highest_;
  boost::posix_time::ptime fo_;
  boost::posix_time::ptime fc_;
};

#endif /* art_Framework_IO_PostCloseFileRenamer_h */

// Local Variables:
// mode: c++
// End:
