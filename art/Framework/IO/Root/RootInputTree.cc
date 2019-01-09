#include "art/Framework/IO/Root/RootInputTree.h"
// vim: set sw=2:

#include "TBranch.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"
#include "art/Framework/IO/Root/RootDelayedReader.h"
#include "art/Framework/IO/Root/detail/dropBranch.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include <cstdint>
#include <string>
#include <utility>

namespace art {

  RootInputTree::RootInputTree(cet::exempt_ptr<TFile> filePtr,
                               BranchType const branchType,
                               int64_t saveMemoryObjectThreshold,
                               cet::exempt_ptr<RootInputFile> primaryFile,
                               bool const compactSubRunRanges,
                               bool const missingOK)
    : filePtr_{filePtr}
    , branchType_{branchType}
    , saveMemoryObjectThreshold_{saveMemoryObjectThreshold}
    , primaryFile_{primaryFile}
    , compactSubRunRanges_{compactSubRunRanges}
  {
    if (filePtr_) {
      tree_ = static_cast<TTree*>(
        filePtr->Get(BranchTypeToProductTreeName(branchType).c_str()));
      metaTree_ = static_cast<TTree*>(
        filePtr->Get(BranchTypeToMetaDataTreeName(branchType).c_str()));
    }
    if (tree_) {
      auxBranch_ =
        tree_->GetBranch(BranchTypeToAuxiliaryBranchName(branchType_).c_str());
      entries_ = tree_->GetEntries();
    }
    if (metaTree_) {
      productProvenanceBranch_ =
        metaTree_->GetBranch(productProvenanceBranchName(branchType_).c_str());
    }
    if (!(missingOK || isValid())) {
      throw Exception(errors::FileReadError)
        << "RootInputTree for branch type " << BranchTypeToString(branchType)
        << " could not be initialized correctly from input file.\n";
    }
  }

  bool
  RootInputTree::isValid() const
  {
    if ((metaTree_ == nullptr) || (metaTree_->GetNbranches() == 0)) {
      return tree_ && auxBranch_ && (tree_->GetNbranches() == 1);
    }
    return tree_ && auxBranch_ && metaTree_ && productProvenanceBranch_;
  }

  bool
  RootInputTree::hasBranch(std::string const& branchName) const
  {
    return tree_->GetBranch(branchName.c_str()) != nullptr;
  }

  void
  RootInputTree::addBranch(BranchKey const& key, BranchDescription const& pd)
  {
    assert(isValid());
    TBranch* branch = tree_->GetBranch(pd.branchName().c_str());
    assert(pd.present() == (branch != nullptr));
    input::BranchInfo info{pd, branch};
    branches_.emplace(key, std::move(info));
  }

  void
  RootInputTree::dropBranch(std::string const& branchName)
  {
    detail::dropBranch(tree_, branchName);
  }

  std::unique_ptr<DelayedReader>
  RootInputTree::makeDelayedReader(
    FileFormatVersion const fileFormatVersion,
    cet::exempt_ptr<BranchIDLists const> branchIDLists,
    BranchType const branchType,
    EntryNumbers const& entrySet,
    EventID const eID)
  {
    return makeDelayedReader(
      fileFormatVersion, nullptr, branchIDLists, branchType, entrySet, eID);
  }

  std::unique_ptr<DelayedReader>
  RootInputTree::makeDelayedReader(
    FileFormatVersion const fileFormatVersion,
    sqlite3* inputDB,
    cet::exempt_ptr<BranchIDLists const> branchIDLists,
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
                                               branchIDLists,
                                               branchType,
                                               eID,
                                               compactSubRunRanges_);
  }

  void
  RootInputTree::setCacheSize(unsigned int cacheSize) const
  {
    tree_->SetCacheSize(static_cast<Long64_t>(cacheSize));
  }

  void
  RootInputTree::setTreeMaxVirtualSize(int treeMaxVirtualSize)
  {
    if (treeMaxVirtualSize >= 0) {
      tree_->SetMaxVirtualSize(static_cast<Long64_t>(treeMaxVirtualSize));
    }
  }

  void
  RootInputTree::setEntryNumber(EntryNumber theEntryNumber)
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
  RootInputTree::makeBranchMapper() const
  {
    return std::make_unique<BranchMapperWithReader>(productProvenanceBranch_,
                                                    entryNumber_);
  }

} // namespace art
