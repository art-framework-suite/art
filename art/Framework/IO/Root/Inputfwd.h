#ifndef art_Framework_IO_Root_Inputfwd_h
#define art_Framework_IO_Root_Inputfwd_h

// ======================================================================
//
// Inputfwd
//
// ======================================================================

#include "Rtypes.h"
#include <map>

// ----------------------------------------------------------------------

class TBranch;
class TTree;

namespace art {
  class BranchDescription;
  class BranchKey;
  class FileFormatVersion;
  class FastCloningInfoProvider;
  class RootInputFile;
  class RootDelayedReader;
  class RootTree;
  class RootInputFileSequence;
  class FileCatalogItem;
  class RootInput;

  namespace input {

    struct BranchInfo
    {
      BranchInfo(BranchDescription const& prod)
        : branchDescription_(prod)
        , productBranch_(0)
      { }

      BranchDescription const& branchDescription_;
      TBranch * productBranch_;
    };  // BranchInfo

    using BranchMap   = std::map<BranchKey const, BranchInfo>;
    using EntryNumber = Long64_t;
    using EntryRange  = std::pair<EntryNumber, EntryNumber>;
    Int_t getEntry(TBranch * branch, EntryNumber entryNumber);
    Int_t getEntry(TTree * tree, EntryNumber entryNumber);

  }  // input
}  // art

// ======================================================================

#endif /* art_Framework_IO_Root_Inputfwd_h */

// Local Variables:
// mode: c++
// End:
