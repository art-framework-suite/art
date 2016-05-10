#ifndef art_Framework_IO_Root_RootOutputClosingCriteria_h
#define art_Framework_IO_Root_RootOutputClosingCriteria_h
// vim: set sw=2:

#include "canvas/Persistency/Provenance/FileIndex.h"
#include <chrono>
#include <limits>

namespace art {

  struct ClosingCriteria {
    unsigned maxFileSize {0x7f000000};
    FileIndex::EntryNumber_t maxEventsPerFile {std::numeric_limits<FileIndex::EntryNumber_t>::max()};
    std::chrono::seconds maxFileAge {std::chrono::seconds::max()};
  };


  bool criteriaMet(ClosingCriteria const& criteria,
                   unsigned fileSize,
                   FileIndex::EntryNumber_t eventsPerFile,
                   std::chrono::seconds fileAge);
}

#endif /* art_Framework_IO_Root_RootOutputClosingCritieria_h */

// Local variables:
// mode: c++
// End:
