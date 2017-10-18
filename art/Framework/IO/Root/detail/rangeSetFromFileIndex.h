#ifndef canvas_Persistency_Provenance_rangeSetFromFileIndex_h
#define canvas_Persistency_Provenance_rangeSetFromFileIndex_h

#include "canvas/Persistency/Provenance/FileIndex.h"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

namespace art {
  namespace detail {
    RangeSet rangeSetFromFileIndex(FileIndex const& fileIndex,
                                   RunID runID,
                                   bool compactRanges);

    RangeSet rangeSetFromFileIndex(FileIndex const& fileIndex,
                                   SubRunID subRunID,
                                   bool compactRanges);
  }
}

#endif
