#ifndef art_Framework_IO_Root_mergeProcessHistories_h
#define art_Framework_IO_Root_mergeProcessHistories_h

#include "canvas/Persistency/Provenance/ProcessHistory.h"

#include <string>
#include <vector>

namespace art {
  using process_names_t = std::vector<std::string>;
  ProcessHistory merge_process_histories(ProcessHistory const& anchor,
                                         ProcessHistory const& new_history);

  // The order in which the new histories are merged is determined
  // based on a hash of the process names.
  ProcessHistory merge_process_histories(
    ProcessHistory const& anchor,
    std::vector<ProcessHistory> const& new_histories);

}

#endif /* art_Framework_IO_Root_mergeProcessHistories_h */

// Local Variables:
// mode: c++
// End:
