#ifndef art_Persistency_Provenance_BranchIDListHelper_h
#define art_Persistency_Provenance_BranchIDListHelper_h
// vim: set sw=2:

#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/BranchListIndex.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"

#include <map>
#include <utility>

namespace art {

class MasterProductRegistry;

class BranchIDListHelper {
public:
  typedef std::pair<BranchListIndex, ProductIndex> IndexPair;
  typedef std::map<BranchID, IndexPair> BranchIDToIndexMap;
public:
  static void updateFromInput(BranchIDLists const& bidlists,
                              std::string const& fileName);
  static void updateRegistries(MasterProductRegistry const& reg);
  static void clearRegistries();
public:
  BranchIDListHelper()
    : branchIDToIndexMap_()
  {
  }

  BranchIDToIndexMap const& branchIDToIndexMap() const
  {
    return branchIDToIndexMap_;
  }

private:
  BranchIDToIndexMap branchIDToIndexMap_;
};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif // art_Persistency_Provenance_BranchIDListHelper_h
