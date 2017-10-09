#ifndef art_Framework_IO_Root_detail_rootFileSizeTools_h
#define art_Framework_IO_Root_detail_rootFileSizeTools_h
//
// Utilties for finding the size on disk of TTrees and TBranches
// Adapted from code given by Philippe Canal, pcanal@fnal.gov.
//
// This is known to work for root 5.34.*

#include "TBranch.h"
#include "TObjArray.h"
#include "TTree.h"

namespace art {
  namespace detail {

    Long64_t GetBasketSize(TObjArray* branches, bool ondisk, bool inclusive);
    Long64_t GetBasketSize(TBranch* b, bool ondisk, bool inclusive);
    Long64_t GetTotalSize(TBranch* br, bool ondisk, bool inclusive);
    Long64_t GetTotalSize(TObjArray* branches, bool ondisk);
    Long64_t GetTotalSize(TTree* t, bool ondisk);
    Long64_t sizeOnDisk(TTree* t);
    Long64_t sizeOnDisk(TBranch* branch, bool inclusive);
    void printBranchSummary(std::ostream& os, TBranch* br);
    void printTreeSummary(std::ostream& os, TTree* t);
  }
} // namespace art

#endif /* art_Framework_IO_Root_detail_rootFileSizeTools_h */

// Local Variables:
// mode: c++
// End:
