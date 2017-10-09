#ifndef art_Framework_Core_GroupSelector_h
#define art_Framework_Core_GroupSelector_h

// ======================================================================
//
// Class GroupSelector. Class for user to select specific groups in event.
//
// ======================================================================

#include "canvas/Persistency/Provenance/ProductList.h"
#include <iosfwd>
#include <string>
#include <vector>

namespace art {
  // defined herein:
  class GroupSelector;
  std::ostream& operator<<(std::ostream& os, const GroupSelector& gs);

  // used herein:
  class BranchDescription;
  class GroupSelectorRules;
}

// ----------------------------------------------------------------------

class art::GroupSelector {
public:
  // N.B.: we assume there are not null pointers in the vector allBranches.
  explicit GroupSelector(GroupSelectorRules const& rules,
                         ProductList const& branchDescriptions);

  bool selected(BranchDescription const& desc) const;

  // Printout intended for debugging purposes.
  void print(std::ostream& os) const;

private:
  // Keep a sorted collection indicating which groups are to be selected.
  std::vector<BranchDescription const*> groupsToSelect_{};

}; // GroupSelector

// ======================================================================

#endif /* art_Framework_Core_GroupSelector_h */

// Local Variables:
// mode: c++
// End:
