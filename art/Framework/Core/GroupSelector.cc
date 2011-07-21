#include "art/Framework/Core/GroupSelector.h"

#include "art/Framework/Core/GroupSelectorRules.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Utilities/Exception.h"
#include "boost/algorithm/string.hpp"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include <algorithm>
#include <cctype>
#include <iterator>
#include <ostream>


using namespace cet;
using namespace std;


namespace art {
  GroupSelector::GroupSelector() : groupsToSelect_(), initialized_(false) {}

  void
  GroupSelector::initialize(GroupSelectorRules const& rules,
                            ProductList const& branchDescriptions) {
    typedef GroupSelectorRules::BranchSelectState BranchSelectState;

    // Get a BranchSelectState for each branch, containing the branch
    // name, with its 'select bit' set to false.
    vector<BranchSelectState> branchstates;
    branchstates.reserve(branchDescriptions.size());
    for (ProductList::const_iterator
           it = branchDescriptions.begin(),
           end = branchDescriptions.end();
         it != end;
         ++it) {
      branchstates.push_back(BranchSelectState(&it->second));
    }

    // Now  apply the rules to  the branchstates, in order.  Each rule
    // can override any previous rule, or all previous rules.
    rules.applyToAll(branchstates);

    // For each of the BranchSelectStates that indicates the branch is
    // to be selected, remember the branch name.  The list of branch
    // names must be sorted, for the implementation of 'selected' to
    // work.
    {
      vector<BranchSelectState>::const_iterator it = branchstates.begin();
      vector<BranchSelectState>::const_iterator end = branchstates.end();
      for (; it != end; ++it) {
          if (it->selectMe) groupsToSelect_.push_back(it->desc->branchName());
      }
      sort_all(groupsToSelect_);
    }
    initialized_ = true;
  }

  bool GroupSelector::selected(BranchDescription const& desc) const {
    if (!initialized_) {
      throw art::Exception(art::errors::LogicError)
        << "GroupSelector::selected() called prematurely\n"
        << "before the product registry has been frozen.\n";
    }
    // We are to select this 'branch' if its name is one of the ones we
    // have been told to select.
    return binary_search_all(groupsToSelect_, desc.branchName());
  }

  void
  GroupSelector::print(ostream& os) const {
    os << "GroupSelector at: "
       << static_cast<void const*>(this)
       << " has "
       << groupsToSelect_.size()
       << " groups to select:\n";
    copy_all(groupsToSelect_, ostream_iterator<string>(os, "\n"));
  }


  //--------------------------------------------------
  //
  // Associated free functions
  //
  ostream&
  operator<< (ostream& os, const GroupSelector& gs)
  {
    gs.print(os);
    return os;
  }

}  // art
