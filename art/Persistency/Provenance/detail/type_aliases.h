#ifndef art_Persistency_Provenance_detail_type_aliases_h
#define art_Persistency_Provenance_detail_type_aliases_h

#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/BranchType.h"

#include <array>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>


namespace art {

  class FileBlock;

  inline namespace process {
           // Used for indices to find branch IDs by branchType, class type and process.
           // ... the key is the process name.
           using ProcessLookup    = std::map<std::string const, std::vector<ProductID>>;
           // ... the key is the friendly class name.
           using TypeLookup       = std::map<std::string const, ProcessLookup>;
           using BranchTypeLookup = std::array<TypeLookup, NumBranchTypes>;

           using ProductListUpdatedCallback = std::function<void(FileBlock const&)>;
  }

  inline namespace presence {
           // Used for determining product presence information in input files
           // FIXME: Change from set to unordered_set?
           using PresenceSet           = std::set<ProductID>;
           using PerBranchTypePresence = std::array<PresenceSet,NumBranchTypes>;
           using PerFilePresence       = std::vector<PerBranchTypePresence>;
  }

}

#endif /* art_Persistency_Provenance_detail_type_aliases_h */

// Local variables:
// mode: c++
// End:
