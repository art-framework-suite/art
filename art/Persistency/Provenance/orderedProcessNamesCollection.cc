#include "art/Persistency/Provenance/orderedProcessNamesCollection.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "boost/algorithm/string.hpp"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"
#include "cetlib/container_algorithms.h"
#include "range/v3/view.hpp"

namespace {
  auto
  stringified_process_names(art::ProcessHistory const& history)
  {
    assert(not history.empty());
    return std::accumulate(
      history.begin() + 1,
      history.end(),
      history.begin()->processName(),
      [](std::string result, art::ProcessConfiguration const& config) {
        result += "\n";
        result += config.processName();
        return result;
      });
  }

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
  // For simplicity, we create a list of all process names,
  // concatenated according to each history.  Although it is possible
  // for two non-overlapping histories to have the same process names,
  // we do not make such a distinction here.
  auto
  collapsed_histories(std::vector<std::string> const& all_process_names)
  {
    assert(not empty(all_process_names));

    std::vector<std::string> result;
    auto candidate = cbegin(all_process_names);
    auto const end = cend(all_process_names);
    for (auto test = candidate + 1; test != end; ++test, ++candidate) {
      if (test->find(*candidate) != 0) {
        result.push_back(*candidate);
      }
    }
    result.push_back(*candidate);
    return result;
  }

  auto
  transform_to_final_result(std::vector<std::string> const& collapsed)
  {
    std::vector<std::vector<std::string>> result;
    for (auto const& process_names_str : collapsed) {
      std::vector<std::string> process_names;
      boost::split(process_names, process_names_str, boost::is_any_of("\n"));
      result.push_back(move(process_names));
    }
    return result;
  }
}

std::vector<std::vector<std::string>>
art::detail::orderedProcessNamesCollection(ProcessHistoryMap const& histories)
{
  std::vector<std::string> all_process_names;
  all_process_names.reserve(histories.size());
  for (auto const& history : histories | ranges::view::values) {
    all_process_names.push_back(stringified_process_names(history));
  }
  cet::sort_all(all_process_names);

  // It is possible for two non-overlapping histories to have the same
  // process name.  We thus need to erase duplicate names.
  auto const e = end(all_process_names);
  auto const new_end = std::unique(begin(all_process_names), e);
  all_process_names.erase(new_end, e);

  auto const collapsed = collapsed_histories(all_process_names);
  return transform_to_final_result(collapsed);
}
