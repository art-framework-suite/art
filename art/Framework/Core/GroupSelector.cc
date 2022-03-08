#include "art/Framework/Core/GroupSelector.h"

#include "art/Framework/Core/GroupSelectorRules.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "cetlib/container_algorithms.h"
#include "range/v3/action/sort.hpp"
#include "range/v3/view.hpp"

#include <ostream>
#include <utility>

using namespace art;
using namespace cet;
using namespace std;

GroupSelector::GroupSelector(GroupSelectorRules const& rules,
                             ProductDescriptionsByID const& descriptions)
{
  using BranchSelectState = GroupSelectorRules::BranchSelectState;

  // Get a BranchSelectState for each branch, containing the branch
  // name, with its 'select bit' set to false.
  auto branchstates = descriptions | ranges::views::values |
                      ranges::views::transform(
                        [](auto const& pd) { return BranchSelectState{&pd}; }) |
                      ranges::to<std::vector>();

  // Now apply the rules to the branchstates, in order.  Each rule can
  // override any previous rule, or all previous rules.
  rules.applyToAll(branchstates);

  // For each of the BranchSelectStates that indicates the branch is
  // to be selected, remember the branch.  The list of branch pointers
  // must be sorted for subsequent binary search to work.
  groupsToSelect_ =
    branchstates |
    ranges::views::filter([](auto const& state) { return state.selectMe; }) |
    ranges::views::transform([](auto const& state) { return state.desc; }) |
    ranges::to<std::vector>();
  sort_all(groupsToSelect_);
}

bool
GroupSelector::selected(BranchDescription const& desc) const
{
  return binary_search_all(groupsToSelect_, &desc);
}

void
GroupSelector::print(ostream& os) const
{
  os << "GroupSelector at: " << static_cast<void const*>(this) << " has "
     << groupsToSelect_.size() << " groups to select:\n";
  for (auto const& bd_ptr : groupsToSelect_) {
    os << bd_ptr->branchName() << '\n';
  }
}

//--------------------------------------------------
// Associated free function

ostream&
art::operator<<(ostream& os, const GroupSelector& gs)
{
  gs.print(os);
  return os;
}

// ======================================================================
