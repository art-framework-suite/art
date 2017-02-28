#ifndef art_Framework_IO_Root_RootOutputTree_h
#define art_Framework_IO_Root_RootOutputTree_h
// vim: set sw=2:

// Used by ROOT output modules.

#include "art/Framework/Core/Frameworkfwd.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "cetlib/exempt_ptr.h"

#include "TTree.h"

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
    static void writeTTree(TTree*);

  public: // MEMBER FUNCTIONS

    // Constructor for trees with no fast cloning
    template<typename T>
    RootOutputTree(/*dummy*/T*,
                   cet::exempt_ptr<TFile> filePtr,
                   BranchType const branchType,
                   typename T::Auxiliary const*& pAux,
                   ProductProvenances*& pProductProvenanceVector,
                   int bufSize,
                   int splitLevel,
                   int64_t treeMaxVirtualSize,
                   int64_t saveMemoryObjectThreshold)
      : filePtr_{filePtr}
      , tree_{makeTTree(filePtr.get(), BranchTypeToProductTreeName(branchType),
                        splitLevel)}
      , metaTree_{makeTTree(filePtr.get(),
                            BranchTypeToMetaDataTreeName(branchType),
                            0)}
      , basketSize_{bufSize}
      , splitLevel_{splitLevel}
      , saveMemoryObjectThreshold_{saveMemoryObjectThreshold}
    {
      if (treeMaxVirtualSize >= 0) {
        tree_->SetMaxVirtualSize(treeMaxVirtualSize);
      }
      auxBranch_ =
        tree_->Branch(BranchTypeToAuxiliaryBranchName(branchType).c_str(),
                      &pAux, bufSize, 0);
      delete pAux;
      pAux = nullptr;
      readBranches_.push_back(auxBranch_);
      productProvenanceBranch_ =
        metaTree_->Branch(productProvenanceBranchName(branchType).c_str(),
                          &pProductProvenanceVector, bufSize, 0);
      metaBranches_.push_back(productProvenanceBranch_);
    }

    RootOutputTree(RootOutputTree const&) = delete;
    RootOutputTree& operator=(RootOutputTree const&) = delete;

    bool
    isValid() const;

    void
    setOutputBranchAddress(BranchDescription const& bd, void const*& pProd);

    void
    resetOutputBranchAddress(BranchDescription const&);

    void
    addOutputBranch(BranchDescription const&, void const*& pProd);

    bool
    checkSplitLevelAndBasketSize(TTree*) const;

    bool fastCloneTree(TTree*);
    void fillTree();
    void writeTree() const;

    TTree*
    tree() const
    {
      return tree_;
    }

    TTree*
    metaTree() const
    {
      return metaTree_;
    }

    void
    setEntries()
    {
      // The member trees are filled by filling their
      // branches individually, which ends up not setting
      // the tree entry count.  Tell the trees to set their
      // entry count based on their branches (all branches
      // must have the same number of entries).
      if (tree_->GetNbranches() != 0) {
        tree_->SetEntries(-1);
      }
      if (metaTree_->GetNbranches() != 0) {
        metaTree_->SetEntries(-1);
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
    TTree* const tree_;
    TTree* const metaTree_;
    TBranch* auxBranch_ {nullptr};
    TBranch* productProvenanceBranch_ {nullptr};
    // does not include cloned branches
    std::vector<TBranch*> producedBranches_ {};
    std::vector<TBranch*> metaBranches_ {};
    std::vector<TBranch*> readBranches_ {};
    std::vector<TBranch*> unclonedReadBranches_ {};
    std::set<std::string> unclonedReadBranchNames_ {};

    // The default for 'fastCloningEnabled_' is false so that SubRuns
    // and Runs are not fast- cloned.  We explicitly set this variable
    // to true for the event tree.
    bool fastCloningEnabled_ {false};

    int basketSize_;
    int splitLevel_;
    int64_t saveMemoryObjectThreshold_;
    int nEntries_ {0};

  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootOutputTree_h */
