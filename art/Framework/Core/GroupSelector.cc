#include "art/Framework/Core/GroupSelector.h"

#include "art/Framework/Core/GroupSelectorRules.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "cetlib/container_algorithms.h"

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
  vector<BranchSelectState> branchstates;
  branchstates.reserve(descriptions.size());
  for (auto const& pr : descriptions) {
    branchstates.push_back(BranchSelectState{&pr.second});
  }

  // Now  apply the rules to  the branchstates, in order.  Each rule
  // can override any previous rule, or all previous rules.
  rules.applyToAll(branchstates);

  // For each of the BranchSelectStates that indicates the branch is
  // to be selected, remember the branch.  The list of branch pointers
  // must be sorted for subsequent binary search to work.
  for (auto const& state : branchstates) {
    if (state.selectMe) {
      groupsToSelect_.push_back(state.desc);
    }
  }
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
