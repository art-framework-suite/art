#include "art/Framework/IO/Root/mergeProcessHistories.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"

#include <iostream>
#include <utility>

namespace {
  auto
  as_str(art::ProcessHistory const& history)
  {
    std::string result;
    for (auto const& pc : history) {
      result += pc.processName();
      result += '\n';
    }
    return result;
  }
}

art::ProcessHistory
art::merge_process_histories(ProcessHistory const& anchors,
                             ProcessHistory const& new_history)
{
  if (new_history.empty() or anchors.empty()) {
    return anchors;
  }

  using iter_t = ProcessHistory::const_iterator;
  std::vector<iter_t> found_names;
  auto const anchor_begin = std::begin(anchors);
  auto const anchor_end = std::end(anchors);
  for (auto const& new_pc : new_history) {
    auto it =
      std::find_if(anchor_begin, anchor_end, [&new_pc](auto const& anchor_pc) {
        return anchor_pc.processName() == new_pc.processName();
      });
    if (it == anchor_end) {
      continue;
    }
    found_names.push_back(it);
  }

  if (not is_sorted(begin(found_names), end(found_names))) {
    throw Exception{errors::MismatchedInputFiles}
      << "The following process histories are inconsistent. They therefore\n"
         "cannot be merged as no well-defined order can be established.\n\n"
      << "History to be merged\n"
      << "--------------------\n"
      << as_str(new_history) << "\nvs.\n\n"
      << "Current merged history\n"
      << "----------------------\n"
      << as_str(anchors) << "\nPlease contact artists@fnal.gov for guidance.";
  }

  // At this point, we can be certain that the new process names are
  // consistent with the anchors.  We can therefore use the following
  // algorithm:
  //
  //   1. For each new process name, check if it is already an anchor
  //      name.
  //   2. If so, then make sure we insert all anchor names up to the
  //      new name.  The anchor iterator must be updated to point to
  //      the unconsumed names.
  //   3. Add the new name.
  //   4. After looping through all new process names, append any
  //      remaining anchor names.
  ProcessHistory result;
  auto anchor_it = anchor_begin;
  for (auto const& new_pc : new_history) {
    auto it =
      std::find_if(anchor_begin, anchor_end, [&new_pc](auto const& anchor_pc) {
        return anchor_pc.processName() == new_pc.processName();
      });
    if (it != anchor_end) {
      std::copy(anchor_it, it, std::back_inserter(result));
      anchor_it = it + 1;
    }
    result.push_back(new_pc);
  }
  std::copy(anchor_it, anchor_end, std::back_inserter(result));
  return result;
}

art::ProcessHistory
art::merge_process_histories(ProcessHistory const& anchor,
                             std::vector<ProcessHistory> const& new_histories)
{
  ProcessHistory result{anchor};

  using iter_t = std::vector<ProcessHistory>::const_iterator;
  std::map<std::string, iter_t> sorted_histories;
  for (auto it = begin(new_histories), e = end(new_histories); it != e; ++it) {
    std::string hash_source;
    for (auto const& pc : *it) {
      hash_source += pc.processName();
      hash_source += ' ';
    }
    sorted_histories.emplace(cet::MD5Digest{hash_source}.digest().toString(),
                             it);
  }

  for (auto const& pr : sorted_histories) {
    result = merge_process_histories(result, *pr.second);
  }
  return result;
}
