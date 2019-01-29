#include "art/Persistency/Provenance/orderedProcessNamesCollection.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"
#include "cetlib/container_algorithms.h"

#include <iomanip>
#include <iostream>
#include <set>

std::vector<std::vector<std::string>>
art::detail::orderedProcessNamesCollection(ProcessHistoryMap const& histories)
{
  std::vector<std::vector<std::string>> result;

  std::vector<ProcessHistory const*> collapsed_histories;
  // Collapsed histories are histories that do not overlap with any
  // others.  For example, of the following process histories:
  //
  //     1: [A]
  //     2: [A,B]
  //     3: [A,B,C]
  //     4: [A,D]
  //
  // The collapsed histories are 3 and 4 since 1 and 2 are subsets of
  // 3.  In otherwords, 3 and 4 are the only histories that do not
  // have descendants.
  //
  // Since the ordering of the histories is determined by a hashed
  // value, we must compare each history in pairs to determine which
  // histories do not have descendants.
  for (auto const& hist_i : histories) {
    bool found_descendent{false};
    for (auto const& hist_j : histories) {
      if (isAncestor(hist_i.second, hist_j.second)) {
        found_descendent = true;
        break;
      }
    }
    if (!found_descendent) {
      collapsed_histories.push_back(&hist_i.second);
    }
  }

  for (auto const history : collapsed_histories) {
    std::vector<std::string> process_names;
    cet::transform_all(*history,
                       std::back_inserter(process_names),
                       [](auto const& config) { return config.processName(); });
    result.push_back(std::move(process_names));
  }

  // It is possible for two non-overlapping histories to have the same
  // process name.  We thus need to erase duplicate names.
  cet::sort_all(result);
  auto const e = end(result);
  auto const new_end = std::unique(begin(result), e);
  result.erase(new_end, e);
  return result;
}
