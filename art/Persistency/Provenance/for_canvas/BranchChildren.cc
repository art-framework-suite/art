#include "canvas/Persistency/Provenance/BranchChildren.h"

#include <utility>

namespace art {

  void
  BranchChildren::append_(map_t const& lookup,
                          BranchID const item,
                          BranchIDSet& itemSet) const
  {
    auto const& items = const_cast<map_t&>(lookup)[item];
    // For each parent(child)
    for (auto const& i : items) {
      // Insert the BranchID of the parents(children) into the set of ancestors(descendants).
      // If the insert succeeds, append recursively.
      if (itemSet.insert(i).second) {
        append_(lookup, i, itemSet);
      }
    }
  }

  void
  BranchChildren::clear()
  {
    childLookup_.clear();
  }

  void
  BranchChildren::insertEmpty(BranchID const parent) {
    childLookup_.emplace(parent, BranchIDSet());
  }

  void
  BranchChildren::insertChild(BranchID const parent, BranchID const child) {
    childLookup_[parent].insert(child);
  }

  void
  BranchChildren::appendToDescendants(BranchID const parent, BranchIDSet& descendants) const {
    descendants.insert(parent);
    append_(childLookup_, parent, descendants);
  }
}
