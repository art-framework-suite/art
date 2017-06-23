#include "art/Framework/IO/Root/RootInputTree.h"
// vim: set sw=2:

#include "art/Framework/IO/Root/RootDelayedReader.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "TBranch.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"
#include <cstdint>
#include <string>
#include <utility>

namespace art {

  RootInputTree::
  RootInputTree(cet::exempt_ptr<TFile> filePtr,
           BranchType const branchType,
           int64_t saveMemoryObjectThreshold,
           cet::exempt_ptr<RootInputFile> primaryFile,
           bool const missingOK)
    : filePtr_{filePtr}
    , branchType_{branchType}
    , saveMemoryObjectThreshold_{saveMemoryObjectThreshold}
    , primaryFile_{primaryFile}
  {
    if (filePtr_) {
      tree_ = static_cast<TTree*>(filePtr->Get(BranchTypeToProductTreeName(branchType).c_str()));
      metaTree_ = static_cast<TTree*>(filePtr->Get(BranchTypeToMetaDataTreeName(branchType).c_str()));
    }
    if (tree_) {
      auxBranch_ = tree_->GetBranch(BranchTypeToAuxiliaryBranchName(branchType_).c_str());
      entries_ = tree_->GetEntries();
    }
    if (metaTree_) {
      productProvenanceBranch_ = metaTree_->GetBranch(productProvenanceBranchName(branchType_).c_str());
    }
    if (!(missingOK || isValid())) {
      throw Exception(errors::FileReadError)
        << "RootInputTree for branch type "
        << BranchTypeToString(branchType)
        << " could not be initialized correctly from input file.\n";
    }
  }

  bool
  RootInputTree::
  isValid() const
  {
    if ((metaTree_ == 0) || (metaTree_->GetNbranches() == 0)) {
      return tree_ && auxBranch_ && (tree_->GetNbranches() == 1);
    }
    return tree_ && auxBranch_ && metaTree_ && productProvenanceBranch_;
  }

  bool
  RootInputTree::
  hasBranch(std::string const& branchName) const
  {
    return tree_->GetBranch(branchName.c_str()) != 0;
  }

  void
  RootInputTree::
  addBranch(BranchKey const& key,
            BranchDescription const& pd,
            std::string const& branchName,
            bool const present)
  {
    assert(isValid());
    TBranch* branch = tree_->GetBranch(branchName.c_str());
    assert(present == (branch != 0));
    input::BranchInfo info(pd);
    info.productBranch_ = 0;
    if (present) {
      info.productBranch_ = branch;
      branchNames_.emplace_back(pd.branchName());
    }
    branches_->emplace(key, info);
  }

  void
  RootInputTree::
  dropBranch(std::string const& branchName)
  {
    TBranch* branch = tree_->GetBranch(branchName.c_str());
    if (!branch) {
      return;
    }
    TObjArray* leaves = tree_->GetListOfLeaves();
    int entries = leaves->GetEntries();
    for (int i = 0; i < entries; ++i) {
      TLeaf* leaf = reinterpret_cast<TLeaf*>((*leaves)[i]);
      if (leaf == nullptr) {
        continue;
      }
      TBranch* br = leaf->GetBranch();
      if (br == nullptr) {
        continue;
      }
      if (br->GetMother() == branch) {
        leaves->Remove(leaf);
      }
    }
    leaves->Compress();
    tree_->GetListOfBranches()->Remove(branch);
    tree_->GetListOfBranches()->Compress();
    delete branch;
  }

  std::unique_ptr<DelayedReader>
  RootInputTree::
  makeDelayedReader(FileFormatVersion const fileFormatVersion,
                    cet::exempt_ptr<BranchIDLists const> branchIDLists,
                    BranchType const branchType,
                    EntryNumbers const& entrySet,
                    EventID const eID)
  {
    return makeDelayedReader(fileFormatVersion, nullptr, branchIDLists, branchType, entrySet, eID);
  }

  std::unique_ptr<DelayedReader>
  RootInputTree::
  makeDelayedReader(FileFormatVersion const fileFormatVersion,
                    sqlite3* inputDB,
                    cet::exempt_ptr<BranchIDLists const> branchIDLists,
                    BranchType const branchType,
                    EntryNumbers const& entrySet,
                    EventID const eID)
  {
    return std::make_unique<RootDelayedReader>(fileFormatVersion,
                                               inputDB,
                                               entrySet,
                                               branches_.get(),
                                               this,
                                               saveMemoryObjectThreshold_,
                                               primaryFile_,
                                               branchIDLists,
                                               branchType,
                                               eID);
  }

  void
  RootInputTree::
  setCacheSize(unsigned int cacheSize) const
  {
    tree_->SetCacheSize(static_cast<Long64_t>(cacheSize));
  }

  void
  RootInputTree::
  setTreeMaxVirtualSize(int treeMaxVirtualSize)
  {
    if (treeMaxVirtualSize >= 0) {
      tree_->SetMaxVirtualSize(static_cast<Long64_t>(treeMaxVirtualSize));
    }
  }

  void
  RootInputTree::
  setEntryNumber(EntryNumber theEntryNumber)
  {
    // Note: An entry number of -1 is ok, this can be used
    //       to put the tree an an invalid entry.
    entryNumber_ = theEntryNumber;
    auto err = tree_->LoadTree(theEntryNumber);
    if (err == -2) {
      // FIXME: Throw an error here!
      // FIXME: -2 means entry number too big.
    }
  }

  std::unique_ptr<BranchMapper>
  RootInputTree::
  makeBranchMapper() const
  {
    return std::make_unique<BranchMapperWithReader>(productProvenanceBranch_, entryNumber_);
  }

} // namespace art
