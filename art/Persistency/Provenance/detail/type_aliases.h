#ifndef art_Persistency_Provenance_detail_type_aliases_h
#define art_Persistency_Provenance_detail_type_aliases_h

#include <array>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>


namespace art {

  class FileBlock;

  inline namespace process {
           // Used for indices to find branch IDs by branchType, class type and process.
           // ... the key is the process name.
           using ProcessLookup    = std::map<std::string const, std::vector<BranchID>>;
           // ... the key is the friendly class name.
           using TypeLookup       = std::map<std::string const, ProcessLookup>;
           using BranchTypeLookup = std::array<TypeLookup, NumBranchTypes>;

           using ProductListUpdatedCallback = std::function<void (FileBlock const &)>;
  }

  inline namespace presence {
           // Used for determining product presence information in input files
           using PresenceSet           = std::unordered_set<BranchID,BranchID::Hash>;
           using PerBranchTypePresence = std::array<PresenceSet,NumBranchTypes>;
           using PerFilePresence       = std::vector<PerBranchTypePresence>;
  }

  inline namespace registration {
           // Used for determining which products were declared (with
           // produces<>) per branch type, and module label
           using BranchSummary_t        = std::string;
           using ProducedMap            = std::unordered_map<BranchID,BranchSummary_t,BranchID::Hash>;
           using PerBranchTypeProduced  = std::array<ProducedMap,NumBranchTypes>;
  }

}

#endif /* art_Persistency_Provenance_detail_type_aliases_h */

// Local variables:
// mode: c++
// End:
