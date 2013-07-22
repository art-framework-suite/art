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
  explicit PostCloseFileRenamer(std::string const & filePattern,
                               std::string const & moduleLabel);

  void recordFileOpen();
  void recordEvent(EventID const & id);
  void recordFileClose();

  void maybeRenameFile(std::string const & inPath); // Rename given file.

  void reset(); // Reset statistics without renaming.

private:
  std::string filePattern_;
  std::string moduleLabel_;
  EventID lowest_;
  EventID highest_;
  boost::posix_time::ptime fo_;
  boost::posix_time::ptime fc_;
};

#endif /* art_Framework_IO_PostCloseFileRenamer_h */

// Local Variables:
// mode: c++
// End:
