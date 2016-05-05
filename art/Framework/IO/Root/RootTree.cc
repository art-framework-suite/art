#include "art/Framework/IO/Root/RootTree.h"
// vim: set sw=2:

#include "art/Framework/IO/Root/RootDelayedReader.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "TBranch.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>

namespace art {

  RootTree::
  RootTree(std::shared_ptr<TFile> filePtr,
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
        << "RootTree for branch type "
        << BranchTypeToString(branchType)
        << " could not be initialized correctly from input file.\n";
    }
  }

  bool
  RootTree::
  isValid() const
  {
    if ((metaTree_ == 0) || (metaTree_->GetNbranches() == 0)) {
      return tree_ && auxBranch_ && (tree_->GetNbranches() == 1);
    }
    return tree_ && auxBranch_ && metaTree_ && productProvenanceBranch_;
  }

  bool
  RootTree::
  hasBranch(std::string const& branchName) const
  {
    return tree_->GetBranch(branchName.c_str()) != 0;
  }

  void
  RootTree::
  addBranch(BranchKey const& key,
            BranchDescription const& bd,
            std::string const& branchName,
            bool const present)
  {
    assert(isValid());
    TBranch* branch = tree_->GetBranch(branchName.c_str());
    assert(present == (branch != 0));
    input::BranchInfo info(bd);
    info.productBranch_ = 0;
    if (present) {
      info.productBranch_ = branch;
      branchNames_.emplace_back(bd.branchName());
    }
    branches_->emplace(key, info);
  }

  void
  RootTree::
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
  RootTree::
  makeDelayedReader(FileFormatVersion const fileFormatVersion,
                    BranchType const branchType,
                    EntryNumbers const& entrySet,
                    EventID const eID)
  {
    return makeDelayedReader(fileFormatVersion, nullptr, branchType, entrySet, eID);
  }

  std::unique_ptr<DelayedReader>
  RootTree::
  makeDelayedReader(FileFormatVersion const fileFormatVersion,
                    sqlite3* inputDB,
                    BranchType const branchType,
                    EntryNumbers const& entrySet,
                    EventID const eID)
  {
    return std::make_unique<RootDelayedReader>(fileFormatVersion,
                                               inputDB,
                                               entrySet,
                                               branches_,
                                               this,
                                               saveMemoryObjectThreshold_,
                                               primaryFile_,
                                               branchType,
                                               eID);
  }

  void
  RootTree::
  setCacheSize(unsigned int cacheSize) const
  {
    tree_->SetCacheSize(static_cast<Long64_t>(cacheSize));
  }

  void
  RootTree::
  setTreeMaxVirtualSize(int treeMaxVirtualSize)
  {
    if (treeMaxVirtualSize >= 0) {
      tree_->SetMaxVirtualSize(static_cast<Long64_t>(treeMaxVirtualSize));
    }
  }

  void
  RootTree::
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
  RootTree::
  makeBranchMapper() const
  {
    return std::make_unique<BranchMapperWithReader>(productProvenanceBranch_, entryNumber_);
  }

  namespace input {

    Int_t
    getEntry(TBranch* branch, EntryNumber entryNumber) try
      {
        return branch->GetEntry(entryNumber);
      }
    catch (cet::exception& e)
      {
        throw art::Exception(art::errors::FileReadError) << e.explain_self() << "\n";
      }

    Int_t
    getEntry(TTree* tree, EntryNumber entryNumber) try
      {
        return tree->GetEntry(entryNumber);
      }
    catch (cet::exception& e)
      {
        throw art::Exception(art::errors::FileReadError) << e.explain_self() << "\n";
      }

  } // namespace input
} // namespace art
