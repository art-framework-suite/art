#ifndef art_Persistency_Provenance_BranchIDListHelper_h
#define art_Persistency_Provenance_BranchIDListHelper_h

#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/BranchListIndex.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "cpp0x/utility"
#include <map>

namespace art {

  class MasterProductRegistry;

  class BranchIDListHelper {
  public:
    typedef std::pair<BranchListIndex, ProductIndex> IndexPair;
    typedef std::map<BranchID, IndexPair> BranchIDToIndexMap;
    BranchIDListHelper() : branchIDToIndexMap_() {}
    static void updateFromInput(BranchIDLists const & bidlists, std::string const & fileName);
    static void updateRegistries(MasterProductRegistry const & reg);
    static void clearRegistries();  // Use only for tests

    BranchIDToIndexMap const & branchIDToIndexMap() const {return branchIDToIndexMap_;}

  private:
    BranchIDToIndexMap branchIDToIndexMap_;
  };
}

#endif /* art_Persistency_Provenance_BranchIDListHelper_h */

// Local Variables:
// mode: c++
// End:
