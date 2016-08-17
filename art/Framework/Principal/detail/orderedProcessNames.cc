#include "art/Framework/Principal/detail/orderedProcessNames.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"
#include "cetlib/container_algorithms.h"

#include <iostream>

std::vector<std::string>
art::detail::orderedProcessNames()
{
  std::vector<std::string> result;
  // Find entry with largest number of process histories
  ProcessHistory history;
  ProcessHistory::size_type max_size {};

  for (auto const& hist : ProcessHistoryRegistry::get()) {
    if (hist.second.size() > max_size) {
      history = hist.second;
      max_size = hist.second.size();
    }
  }

  // The process configurations are stored in order, so using
  // back-insertion is appropriate.
  cet::transform_all(history,
                     std::back_inserter(result),
                     [](auto const& config) {
                       return config.processName();
                     } );

  return result;
}
