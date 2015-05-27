#include "art/Framework/IO/Root/RootOutputTree.h"
// vim: set sw=2:

#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "Rtypes.h"
#include "TClass.h"
#include "TBranch.h"
#include "TFile.h"
#include "TTreeCloner.h"

#include <functional>
#include <iostream>
#include <limits>

using namespace cet;
using namespace std;

namespace art {

TTree*
RootOutputTree::
makeTTree(TFile* filePtr, string const& name, int splitLevel)
{
  TTree* tree = new TTree(name.c_str(), "", splitLevel);
  if (!tree) {
    throw art::Exception(art::errors::FatalRootError)
        << "Failed to create the tree: "
        << name
        << "\n";
  }
  if (tree->IsZombie()) {
    throw art::Exception(art::errors::FatalRootError)
        << "Tree: "
        << name
        << " is a zombie.\n";
  }
  tree->SetDirectory(filePtr);
  // Turn off autosave because it leaves too many deleted tree
  // keys in the output file.
  tree->SetAutoSave(numeric_limits<Long64_t>::max());
  return tree;
}

bool
RootOutputTree::
checkSplitLevelAndBasketSize(TTree* inputTree) const
{
  // Do the split level and basket size match in the input and output?
  if (inputTree == 0) {
    return false;
  }
  for (auto const& val : readBranches_) {
    TBranch* outputBranch = val;
    if (outputBranch == 0) {
      continue;
    }
    TBranch* inputBranch = inputTree->GetBranch(outputBranch->GetName());
    if (inputBranch == 0) {
      continue;
    }
    if ((inputBranch->GetSplitLevel() != outputBranch->GetSplitLevel()) ||
        (inputBranch->GetBasketSize() != outputBranch->GetBasketSize())) {
      mf::LogInfo("FastCloning")
          << "Fast Cloning disabled because split level or basket size "
          "do not match";
      return false;
    }
  }
  return true;
}

void
RootOutputTree::
writeTTree(TTree* tree)
{
  // Update the tree-level entry count because we have been
  // using branch fill instead of tree fill.
  if (tree->GetNbranches() != 0) {
    tree->SetEntries(-1);
  }
  // Use auto save here instead of write because it deletes
  // the old tree key from the file, does not flush the
  // baskets, and writes out the streamer infos, unlike write.
  tree->AutoSave();
}

void
RootOutputTree::
writeTree() const
{
  writeTTree(tree_);
  writeTTree(metaTree_);
}

static
void
fastCloneTTree(TTree* in, TTree* out)
{
  if (in->GetEntries() == 0) {
    return;
  }
  TTreeCloner cloner(in, out, "", TTreeCloner::kIgnoreMissingTopLevel);
  if (!cloner.IsValid()) {
    throw art::Exception(art::errors::FatalRootError)
        << "invalid TTreeCloner\n";
  }
  out->SetEntries(out->GetEntries() + in->GetEntries());
  cloner.Exec();
}

void
RootOutputTree::
fastCloneTree(TTree* tree)
{
  unclonedReadBranches_.clear();
  unclonedReadBranchNames_.clear();
  if (!currentlyFastCloning_) {
    return;
  }
  fastCloneTTree(tree, tree_);
  for (auto const& val : readBranches_) {
    if (val->GetEntries() != tree_->GetEntries()) {
      unclonedReadBranches_.push_back(val);
      unclonedReadBranchNames_.insert(string(val->GetName()));
    }
  }
}

static
void
fillTreeBranches(TTree*, vector<TBranch*> const& branches,
                 bool saveMemory, int64_t threshold)
{
  for (auto const b : branches) {
    auto bytesWritten = b->Fill();
    if (saveMemory && (bytesWritten > threshold)) {
      b->FlushBaskets();
      b->DropBaskets("all");
    }
  }
}

void
RootOutputTree::
fillTree()
{
  fillTreeBranches(metaTree_, metaBranches_, false,
                   saveMemoryObjectThreshold_);
  bool saveMemory = (saveMemoryObjectThreshold_ > -1);
  fillTreeBranches(tree_, producedBranches_, saveMemory,
                   saveMemoryObjectThreshold_);
  if (currentlyFastCloning_) {
    fillTreeBranches(tree_, unclonedReadBranches_, saveMemory,
                     saveMemoryObjectThreshold_);
  }
  else {
    fillTreeBranches(tree_, readBranches_, saveMemory,
                     saveMemoryObjectThreshold_);
  }
  ++nEntries_;
}

void
RootOutputTree::
resetOutputBranchAddress(BranchDescription const& bd)
{
  TBranch* br = tree_->GetBranch(bd.branchName().c_str());
  if (br == nullptr) {
    return;
  }
  tree_->ResetBranchAddress(br);
}

void
RootOutputTree::
setOutputBranchAddress(BranchDescription const& bd, void const*& pProd)
{
  if (TBranch* br = tree_->GetBranch(bd.branchName().c_str())) {
    br->SetAddress(&pProd);
  }
}

void
RootOutputTree::
addOutputBranch(BranchDescription const& bd, void const*& pProd)
{
  if (TBranch* br = tree_->GetBranch(bd.branchName().c_str())) {
    // Already have this branch, possibly update the branch address.
    if (pProd == nullptr) {
      // The OutputItem is freshly constructed and has
      // not been passed to SetAddress yet.
      // If selectProducts has just been called, we
      // get here just after the branch object has been
      // deleted with a ResetBranchAddress() to prepare
      // for the OutputItem being replaced, and the
      // OutputItem has just been recreated.
      TClass* cls = TClass::GetClass(bd.wrappedCintName().c_str());
      EDProduct* prod = reinterpret_cast<EDProduct*>(cls->New());
      pProd = prod;
      br->SetAddress(&pProd);
      pProd = nullptr;
      delete prod;
    }
    return;
  }
  auto bsize = bd.basketSize();
  if (bsize == BranchDescription::invalidBasketSize) {
    bsize = basketSize_;
  }
  auto splitlvl = bd.splitLevel();
  if (splitlvl == BranchDescription::invalidSplitLevel) {
    splitlvl = splitLevel_;
  }
  if (pProd != nullptr) {
    throw art::Exception(art::errors::FatalRootError)
        << "OutputItem product pointer is not nullptr!\n";
  }
  TClass* cls = TClass::GetClass(bd.wrappedCintName().c_str());
  EDProduct* prod = reinterpret_cast<EDProduct*>(cls->New());
  pProd = prod;
  TBranch* branch = tree_->Branch(bd.branchName().c_str(),
                                  bd.wrappedCintName().c_str(),
                                  &pProd, bsize, splitlvl);
  //TBranch* branch = tree_->Branch(bd.branchName().c_str(),
  //                                bd.wrappedCintName().c_str(),
  //                                nullptr, bsize, splitlvl);
  // Note that root will have just allocated a dummy product
  // as the I/O buffer for the branch we have created.  We will
  // replace this I/O buffer in RootOutputFile::fillBranches()
  // with the actual product or our own dummy using
  // TBranchElement::SetAddress(), which will cause root to
  // automatically delete the dummy product it allocated here.
  //if (pProd == nullptr) {
  //  throw art::Exception(art::errors::FatalRootError)
  //      << "OutputItem product pointer is a nullptr, branch"
  //      << "creation failed to allocate an I/O buffer!\n";
  //}
  pProd = nullptr;
  delete prod;
  if (bd.compression() != BranchDescription::invalidCompression) {
    branch->SetCompressionSettings(bd.compression());
  }
  if (nEntries_ > 0) {
    // Backfill the branch with dummy entries to match the number
    // of entries already written to the data tree.
    TClass* c = TClass::GetClass(bd.wrappedCintName().c_str());
    std::unique_ptr<EDProduct> dummy(static_cast<EDProduct*>(c->New()));
    pProd = dummy.get();
    int bytesWritten = 0;
    for (auto i = nEntries_; i > 0; --i) {
      auto cnt = branch->Fill();
      if (cnt <= 0) {
        // FIXME: Throw a fatal error here!
      }
      bytesWritten += cnt;
      if ((saveMemoryObjectThreshold_ > -1) &&
          (bytesWritten > saveMemoryObjectThreshold_)) {
        branch->FlushBaskets();
        branch->DropBaskets("all");
      }
    }
  }
  if (bd.produced()) {
    producedBranches_.push_back(branch);
  }
  else {
    readBranches_.push_back(branch);
  }
}

} // namespace art

