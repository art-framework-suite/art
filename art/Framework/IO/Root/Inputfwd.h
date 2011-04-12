#ifndef art_Framework_IO_Root_Inputfwd_h
#define art_Framework_IO_Root_Inputfwd_h

// ======================================================================
//
// Inputfwd
//
// ======================================================================

#include "Reflex/Type.h"
#include "Rtypes.h"
#include "art/Persistency/Provenance/ConstBranchDescription.h"
#include <map>

// ----------------------------------------------------------------------

class TBranch;
class TTree;

namespace art {
  class BranchKey;
  class FileFormatVersion;
  class RootInputFile;
  class RootDelayedReader;
  class RootTree;
  class RootInputFileSequence;
  class FileCatalogItem;
  class RootInput;

  namespace input {

    struct BranchInfo
    {
      BranchInfo(ConstBranchDescription const& prod)
        : branchDescription_(prod)
        , productBranch_(0)
      { }

      ConstBranchDescription branchDescription_;
      TBranch * productBranch_;
    };  // BranchInfo

    typedef std::map<BranchKey const, BranchInfo> BranchMap;
    typedef Long64_t EntryNumber;
    Int_t getEntry(TBranch * branch, EntryNumber entryNumber);
    Int_t getEntry(TTree * tree, EntryNumber entryNumber);

  }  // input
}  // art

// ======================================================================

#endif /* art_Framework_IO_Root_Inputfwd_h */

// Local Variables:
// mode: c++
// End:
