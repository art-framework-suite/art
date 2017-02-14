#ifndef art_Persistency_Provenance_BranchIDListRegistry_h
#define art_Persistency_Provenance_BranchIDListRegistry_h

//=====================================================================
// BranchIDListRegistry: This is a thread-UNSAFE registry.  Access to
//                       it must be, and is intended to be, serialized.
// ====================================================================

#include "canvas/Persistency/Provenance/BranchIDList.h"
#include "canvas/Persistency/Provenance/BranchListIndex.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"

#include <map>
#include <vector>
#include <utility>

namespace art {

  class MasterProductRegistry;

  class BranchIDListRegistry {
  public:

    using collection_type = std::vector<BranchIDList>;

    auto size() const { return data_.size(); }
    auto const& data() const { return data_; }
    auto const& branchIDToIndexMap() const { return branchIDToIndexMap_; }

    static BranchIDListRegistry& instance();
    static void updateFromInput(BranchIDLists const& file_bidlists, std::string const& fileName);
    static void updateFromProductRegistry(MasterProductRegistry const& reg);

    using IndexPair = std::pair<BranchListIndex, ProductIndex>;
    using BranchIDToIndexMap = std::map<BranchID, IndexPair>;

  private:

    BranchIDListRegistry() = default;
    ~BranchIDListRegistry() = default;

    static void generate_branchIDToIndexMap();

    // Not copyable:
    BranchIDListRegistry(BranchIDListRegistry const&) = delete;
    BranchIDListRegistry& operator= (BranchIDListRegistry const&) = delete;

    collection_type data_ {};
    BranchIDToIndexMap branchIDToIndexMap_ {};

    static BranchIDListRegistry* instance_;
  };

}  // art

// Local Variables:
// mode: c++
// End:
#endif /* art_Utilities_BranchIDListRegistry_h */
