#include "art/Framework/IO/Root/detail/orderedProcessNames.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"
#include "cetlib/container_algorithms.h"

std::vector<std::string>
art::detail::orderedProcessNames(std::string const& currentProcessName)
{
  std::vector<std::string> result;
  // Find entry with largest number of process histories
  ProcessHistory history;
  ProcessHistory::size_type max_size {};

  if (ProcessHistoryRegistry::empty()) {
    result.push_back(currentProcessName);
    return result;
  }

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

  assert(!result.empty());
  if (result.back() != currentProcessName)
    result.push_back(currentProcessName);

  return result;
}
