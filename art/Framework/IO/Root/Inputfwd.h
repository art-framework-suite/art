#ifndef art_Framework_IO_Root_Inputfwd_h
#define art_Framework_IO_Root_Inputfwd_h
// vim: set sw=2 expandtab :

#include "Rtypes.h"
#include "hep_concurrency/RecursiveMutex.h"

#include <map>
#include <vector>

class TBranch;
class TTree;

namespace art {

  class BranchDescription;
  struct FileFormatVersion;
  class FastCloningInfoProvider;
  class ProductID;
  class RootInputFile;
  class RootDelayedReader;
  class RootInputTree;
  class RootInputFileSequence;
  class FileCatalogItem;
  class RootInput;

  namespace input {

    struct BranchInfo {
      BranchInfo(BranchDescription const& prod, TBranch* const branch)
        : branchDescription_{prod}, productBranch_{branch}
      {}

      // Ideally, a reference to the branch-description does not need
      // to be retained.  It is used to fill the groups in the
      // principal.
      BranchDescription const& branchDescription_;
      TBranch* productBranch_;
    };

    using BranchMap = std::map<ProductID, BranchInfo>;
    using EntryNumber = Long64_t;
    using EntryNumbers = std::vector<EntryNumber>;
    Int_t getEntry(TBranch* branch, EntryNumber entryNumber);
    Int_t getEntry(TBranch* branch,
                   EntryNumber entryNumber,
                   unsigned long long& ticks);
    Int_t getEntry(TTree* tree, EntryNumber entryNumber);
    Int_t getEntry(TTree* tree,
                   EntryNumber entryNumber,
                   unsigned long long& ticks);

    class RootMutexSentry {
      friend class AutoRootMutexSentryShutdown;

    private:
      static hep::concurrency::RecursiveMutex* rootMutex_;

    public:
      static hep::concurrency::RecursiveMutex* startup();
      static void shutdown();

    public:
      ~RootMutexSentry();
      RootMutexSentry();
      RootMutexSentry(RootMutexSentry const&) = delete;
      RootMutexSentry& operator=(RootMutexSentry const&) = delete;

    private:
      hep::concurrency::RecursiveMutexSentry sentry_;
    };

  } // namespace input
} // namespace art

#endif /* art_Framework_IO_Root_Inputfwd_h */

// Local Variables:
// mode: c++
// End:
