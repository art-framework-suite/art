#ifndef art_Framework_IO_Root_RootOutputClosingCriteria_h
#define art_Framework_IO_Root_RootOutputClosingCriteria_h
// vim: set sw=2:

#include "art/Persistency/Provenance/FileIndex.h"
#include <limits>

namespace art {
  struct ClosingCriteria {
    unsigned maxFileSize {0x7f000000};
    FileIndex::EntryNumber_t maxEventsPerFile {std::numeric_limits<FileIndex::EntryNumber_t>::max()};
  };


  bool criteriaMet(ClosingCriteria const& criteria,
                   unsigned maxFileSize,
                   FileIndex::EntryNumber_t maxEventsPerFile);
}

#endif /* art_Framework_IO_Root_RootOutputClosingCritieria_h */

// Local variables:
// mode: c++
// End:
