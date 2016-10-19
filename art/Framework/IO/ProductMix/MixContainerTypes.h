#ifndef art_Framework_IO_ProductMix_MixContainerTypes_h
#define art_Framework_IO_ProductMix_MixContainerTypes_h

#include "canvas/Persistency/Provenance/FileIndex.h"

#include <map>
#include <vector>

namespace art {
  class EventAuxiliary;
  class EventID;

  typedef std::map<FileIndex::EntryNumber_t, EventID> EventIDIndex;
  typedef std::vector<EventID> EventIDSequence;
  typedef std::vector<FileIndex::EntryNumber_t> EntryNumberSequence;
  typedef std::vector<EventAuxiliary> EventAuxiliarySequence;
}

#endif /* art_Framework_IO_ProductMix_MixContainerTypes_h */

// Local Variables:
// mode: c++
// End:
