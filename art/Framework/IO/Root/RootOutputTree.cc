#include "art/Framework/IO/Root/RootOutputTree.h"
// vim: set sw=2:

#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "Rtypes.h"
#include "TBranch.h"
#include "TClass.h"
#include "TClassRef.h"
#include "TFile.h"
#include "TTreeCloner.h"

#include <functional>
#include <limits>

using namespace cet;
using namespace std;

namespace art {

  TTree*
  RootOutputTree::makeTTree(TFile* filePtr, string const& name, int splitLevel)
  {
    auto tree = new TTree{name.c_str(), "", splitLevel};
    if (!tree) {
      throw art::Exception(art::errors::FatalRootError)
        << "Failed to create the tree: " << name << "\n";
    }
    if (tree->IsZombie()) {
      throw art::Exception(art::errors::FatalRootError)
        << "Tree: " << name << " is a zombie.\n";
    }
    tree->SetDirectory(filePtr);
    // Turn off autosave because it leaves too many deleted tree
    // keys in the output file.
    tree->SetAutoSave(numeric_limits<Long64_t>::max());
    return tree;
  }

  bool
  RootOutputTree::checkSplitLevelAndBasketSize(
    cet::exempt_ptr<TTree const> inputTree) const
  {
    // Do the split level and basket size match in the input and output?
    if (inputTree == nullptr) {
      return false;
    }
    for (auto outputBranch : readBranches_) {
      if (outputBranch == nullptr) {
        continue;
      }
      TBranch* inputBranch =
        const_cast<TTree*>(inputTree.get())->GetBranch(outputBranch->GetName());
      if (inputBranch == nullptr) {
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
  RootOutputTree::writeTTree(TTree* tree) noexcept(false)
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
  RootOutputTree::writeTree() const
  {
    writeTTree(tree_);
    writeTTree(metaTree_);
  }

  bool
  RootOutputTree::fastCloneTree(cet::exempt_ptr<TTree const> intree)
  {
    unclonedReadBranches_.clear();
    unclonedReadBranchNames_.clear();
    if (!fastCloningEnabled_) {
      return false;
    }

    bool cloned{false};
    if (intree->GetEntries() != 0) {
      TTreeCloner cloner(const_cast<TTree*>(intree.get()),
                         tree_,
                         "",
                         TTreeCloner::kIgnoreMissingTopLevel |
                           TTreeCloner::kNoWarnings |
                           TTreeCloner::kNoFileCache);
      if (cloner.IsValid()) {
        tree_->SetEntries(tree_->GetEntries() + intree->GetEntries());
        cloner.Exec();
        cloned = true;
      } else {
        fastCloningEnabled_ = false;
        mf::LogInfo("fastCloneTree")
          << "INFO: Unable to fast clone tree " << intree->GetName() << '\n'
          << "INFO: ROOT reason is:\n"
          << "INFO: " << cloner.GetWarning() << '\n'
          << "INFO: Processing will continue, tree will be slow cloned.";
      }
    }
    for (auto const& val : readBranches_) {
      if (val->GetEntries() != tree_->GetEntries()) {
        unclonedReadBranches_.push_back(val);
        unclonedReadBranchNames_.insert(string(val->GetName()));
      }
    }
    return cloned;
  }

  static void
  fillTreeBranches(TTree*,
                   vector<TBranch*> const& branches,
                   bool saveMemory,
                   int64_t threshold)
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
  RootOutputTree::fillTree()
  {
    fillTreeBranches(
      metaTree_, metaBranches_, false, saveMemoryObjectThreshold_);
    bool saveMemory = (saveMemoryObjectThreshold_ > -1);
    fillTreeBranches(
      tree_, producedBranches_, saveMemory, saveMemoryObjectThreshold_);
    if (fastCloningEnabled_) {
      fillTreeBranches(
        tree_, unclonedReadBranches_, saveMemory, saveMemoryObjectThreshold_);
    } else {
      fillTreeBranches(
        tree_, readBranches_, saveMemory, saveMemoryObjectThreshold_);
    }
    ++nEntries_;
  }

  void
  RootOutputTree::resetOutputBranchAddress(BranchDescription const& pd)
  {
    TBranch* br = tree_->GetBranch(pd.branchName().c_str());
    if (br == nullptr) {
      return;
    }
    tree_->ResetBranchAddress(br);
  }

  void
  RootOutputTree::setOutputBranchAddress(BranchDescription const& pd,
                                         void const*& pProd)
  {
    if (TBranch* br = tree_->GetBranch(pd.branchName().c_str())) {
      br->SetAddress(&pProd);
    }
  }

  void
  RootOutputTree::addOutputBranch(BranchDescription const& pd,
                                  void const*& pProd)
  {
    TClassRef cls = TClass::GetClass(pd.wrappedName().c_str());
    if (auto br = tree_->GetBranch(pd.branchName().c_str())) {
      // Already have this branch, possibly update the branch address.
      if (pProd == nullptr) {
        // The OutputItem is freshly constructed and has not been
        // passed to SetAddress yet.  If selectProducts has just been
        // called, we get here just after the branch object has been
        // deleted with a ResetBranchAddress() to prepare for the
        // OutputItem being replaced, and the OutputItem has just been
        // recreated.
        auto prod = reinterpret_cast<EDProduct*>(cls->New());
        pProd = prod;
        br->SetAddress(&pProd);
        pProd = nullptr;
        delete prod;
      }
      return;
    }
    auto bsize = pd.basketSize();
    if (bsize == BranchDescription::invalidBasketSize) {
      bsize = basketSize_;
    }
    auto splitlvl = pd.splitLevel();
    if (splitlvl == BranchDescription::invalidSplitLevel) {
      splitlvl = splitLevel_;
    }
    if (pProd != nullptr) {
      throw art::Exception(art::errors::FatalRootError)
        << "OutputItem product pointer is not nullptr!\n";
    }
    auto prod = reinterpret_cast<EDProduct*>(cls->New());
    pProd = prod;
    TBranch* branch = tree_->Branch(pd.branchName().c_str(),
                                    pd.wrappedName().c_str(),
                                    &pProd,
                                    bsize,
                                    splitlvl);

    // Note that root will have just allocated a dummy product as the
    // I/O buffer for the branch we have created.  We will replace
    // this I/O buffer in RootOutputFile::fillBranches() with the
    // actual product or our own dummy using
    // TBranchElement::SetAddress(), which will cause root to
    // automatically delete the dummy product it allocated here.

    pProd = nullptr;
    delete prod;
    if (pd.compression() != BranchDescription::invalidCompression) {
      branch->SetCompressionSettings(pd.compression());
    }
    if (nEntries_ > 0) {
      // Backfill the branch with dummy entries to match the number
      // of entries already written to the data tree.
      std::unique_ptr<EDProduct> dummy(static_cast<EDProduct*>(cls->New()));
      pProd = dummy.get();
      int bytesWritten{};
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
    if (pd.produced()) {
      producedBranches_.push_back(branch);
    } else {
      readBranches_.push_back(branch);
    }
  }

} // namespace art
