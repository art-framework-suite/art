#ifndef art_Framework_IO_Root_RootOutputTree_h
#define art_Framework_IO_Root_RootOutputTree_h
// vim: set sw=2:

// Used by ROOT output modules.

#include "art/Framework/Core/Frameworkfwd.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "cetlib/exempt_ptr.h"

#include "TTree.h"

#include <atomic>
#include <memory>
#include <set>
#include <string>
#include <vector>

class TFile;
class TBranch;

namespace art {
  class RootOutputTree {
  public: // STATIC MEMBER FUNCTIONS
    static TTree* makeTTree(TFile*, std::string const& name, int splitLevel);
    // This routine MAY THROW if art converts
    // a ROOT error message to an exception.
    static void writeTTree(TTree*) noexcept(false);

  public: // MEMBER FUNCTIONS -- Special Member Functions
    // Constructor for trees with no fast cloning
    template <typename Aux>
    RootOutputTree(cet::exempt_ptr<TFile> filePtr,
                   BranchType const branchType,
                   Aux const*& pAux,
                   ProductProvenances*& pProductProvenanceVector,
                   int const bufSize,
                   int const splitLevel,
                   int64_t const treeMaxVirtualSize,
                   int64_t const saveMemoryObjectThreshold)
      : filePtr_{filePtr}
    {
      tree_ = makeTTree(
        filePtr.get(), BranchTypeToProductTreeName(branchType), splitLevel);
      metaTree_ =
        makeTTree(filePtr.get(), BranchTypeToMetaDataTreeName(branchType), 0);
      fastCloningEnabled_ = false;
      basketSize_ = bufSize;
      splitLevel_ = splitLevel;
      saveMemoryObjectThreshold_ = saveMemoryObjectThreshold;
      nEntries_ = 0;
      if (treeMaxVirtualSize >= 0) {
        tree_.load()->SetMaxVirtualSize(treeMaxVirtualSize);
      }
      auto auxBranch = tree_.load()->Branch(
        BranchTypeToAuxiliaryBranchName(branchType).c_str(), &pAux, bufSize, 0);
      delete pAux;
      pAux = nullptr;
      readBranches_.push_back(auxBranch);
      auto productProvenanceBranch = metaTree_.load()->Branch(
        productProvenanceBranchName(branchType).c_str(),
        &pProductProvenanceVector,
        bufSize,
        0);
      metaBranches_.push_back(productProvenanceBranch);
    }
    RootOutputTree(RootOutputTree const&) = delete;
    RootOutputTree& operator=(RootOutputTree const&) = delete;

  public: // MEMBER FUNCTIONS -- API
    bool isValid() const;
    void resetOutputBranchAddress(BranchDescription const&);
    void addOutputBranch(BranchDescription const&, void const*& pProd);
    bool checkSplitLevelAndBasketSize(cet::exempt_ptr<TTree const>) const;
    bool fastCloneTree(cet::exempt_ptr<TTree const>);
    void fillTree();
    void writeTree() const;
    TTree*
    tree() const
    {
      return tree_.load();
    }
    TTree*
    metaTree() const
    {
      return metaTree_.load();
    }
    void
    setEntries()
    {
      // The member trees are filled by filling their
      // branches individually, which ends up not setting
      // the tree entry count.  Tell the trees to set their
      // entry count based on their branches (all branches
      // must have the same number of entries).
      if (tree_.load()->GetNbranches() != 0) {
        tree_.load()->SetEntries(-1);
      }
      if (metaTree_.load()->GetNbranches() != 0) {
        metaTree_.load()->SetEntries(-1);
      }
    }
    void
    beginInputFile(bool fastCloning)
    {
      fastCloningEnabled_ = fastCloning;
    }
    bool
    uncloned(std::string const& branchName) const
    {
      return unclonedReadBranchNames_.find(branchName) !=
             unclonedReadBranchNames_.end();
    }

  private: // MEMBER DATA
    cet::exempt_ptr<TFile> filePtr_;
    std::atomic<TTree*> tree_;
    std::atomic<TTree*> metaTree_;
    // does not include cloned branches
    std::vector<TBranch*> producedBranches_{};
    std::vector<TBranch*> metaBranches_{};
    std::vector<TBranch*> readBranches_{};
    std::vector<TBranch*> unclonedReadBranches_{};
    std::set<std::string> unclonedReadBranchNames_{};
    // The default for 'fastCloningEnabled_' is false so that SubRuns
    // and Runs are not fast-cloned.  We explicitly set this variable
    // to true for the event tree.
    std::atomic<bool> fastCloningEnabled_;
    std::atomic<int> basketSize_;
    std::atomic<int> splitLevel_;
    std::atomic<int64_t> saveMemoryObjectThreshold_;
    std::atomic<int> nEntries_;
  };
} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootOutputTree_h */
