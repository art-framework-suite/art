#ifndef IOPool_Input_Inputfwd_h
#define IOPool_Input_Inputfwd_h


#include "art/Persistency/Provenance/ConstBranchDescription.h"

#include "Rtypes.h"
#include "Reflex/Type.h"

class TBranch;
class TTree;

#include <map>


namespace edm {
  class BranchKey;
  class FileFormatVersion;
  class RootFile;
  class RootDelayedReader;
  class RootTree;

  namespace input {

    struct BranchInfo {
      BranchInfo(ConstBranchDescription const& prod) :
        branchDescription_(prod),
        productBranch_(0),
        provenanceBranch_(0) {}
      ConstBranchDescription branchDescription_;
      TBranch * productBranch_;
      // The rest are for backward compatibility
      TBranch * provenanceBranch_;
    };
    typedef std::map<BranchKey const, BranchInfo> BranchMap;
    typedef Long64_t EntryNumber;
    Int_t getEntry(TBranch * branch, EntryNumber entryNumber);
    Int_t getEntry(TTree * tree, EntryNumber entryNumber);

  }  // namespace input
}  // namespace edm

#endif  // IOPool_Input_Inputfwd_h
