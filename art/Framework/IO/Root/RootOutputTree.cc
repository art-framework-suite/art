#include "art/Framework/IO/Root/RootOutputTree.h"

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cpp0x/functional"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <limits>

#include "Rtypes.h"
#include "TBranch.h"
#include "TFile.h"
#include "TTreeCloner.h"
#include <iostream>

using namespace cet;
using namespace std;


namespace art {

  TTree *
  RootOutputTree::assignTTree(TFile * filePtr, TTree * tree) {
    tree->SetDirectory(filePtr);
    // Turn off autosaving because it is such a memory hog and we are not using
    // this check-pointing feature anyway.
    tree->SetAutoSave(numeric_limits<Long64_t>::max());
    return tree;
  }

  TTree *
  RootOutputTree::makeTTree(TFile * filePtr, string const& name, int splitLevel) {
    TTree *tree = new TTree(name.c_str(), "", splitLevel);
    if (!tree) throw art::Exception(art::errors::FatalRootError)
      << "Failed to create the tree: " << name << "\n";
    if (tree->IsZombie())
      throw art::Exception(art::errors::FatalRootError)
        << "Tree: " << name << " is a zombie." << "\n";

    return assignTTree(filePtr, tree);
  }

  bool RootOutputTree::checkSplitLevelAndBasketSize(TTree *inputTree) const {

    if (inputTree == 0) return false;

    // Do the split level and basket size match in the input and output?
    for (vector<TBranch *>::const_iterator it = readBranches_.begin(), itEnd = readBranches_.end();
      it != itEnd; ++it) {

      TBranch* outputBranch = *it;
      if (outputBranch != 0) {
        TBranch* inputBranch = inputTree->GetBranch(outputBranch->GetName());

        if (inputBranch != 0) {
          if (inputBranch->GetSplitLevel() != outputBranch->GetSplitLevel() ||
              inputBranch->GetBasketSize() != outputBranch->GetBasketSize()) {
            mf::LogInfo("FastCloning")
              << "Fast Cloning disabled because split level or basket size do not match";
            return false;
          }
        }
      }
    }
    return true;
  }


  void
  RootOutputTree::fastCloneTTree(TTree *in, TTree *out) {
    if (in->GetEntries() != 0) {
      TTreeCloner cloner(in, out, "", TTreeCloner::kIgnoreMissingTopLevel);
      if (!cloner.IsValid()) {
        throw art::Exception(art::errors::FatalRootError)
          << "invalid TTreeCloner\n";
      }
      out->SetEntries(out->GetEntries() + in->GetEntries());
      cloner.Exec();
    }
  }

  void
  RootOutputTree::writeTTree(TTree *tree) {
    if (tree->GetNbranches() != 0) {
      tree->SetEntries(-1);
    }
    tree->AutoSave();
  }

  void
  RootOutputTree::fillTTree(TTree *,
                            vector<TBranch *> const& branches,
                            bool saveMemory) const {
    for (auto const b : branches) {
      auto bytesWritten = b->Fill();
      if (saveMemory &&
          bytesWritten > saveMemoryObjectThreshold_) {
        b->FlushBaskets();
        b->DropBaskets("all");
      }
    }
  }

  void
  RootOutputTree::writeTree() const {
    writeTTree(tree_);
    writeTTree(metaTree_);
  }

  void
  RootOutputTree::fastCloneTree(TTree *tree) {
    unclonedReadBranches_.clear();
    unclonedReadBranchNames_.clear();
    if (currentlyFastCloning_) {
      fastCloneTTree(tree, tree_);
      for (vector<TBranch *>::const_iterator it = readBranches_.begin(), itEnd = readBranches_.end();
          it != itEnd; ++it) {
        if ((*it)->GetEntries() != tree_->GetEntries()) {
          unclonedReadBranches_.push_back(*it);
          unclonedReadBranchNames_.insert(string((*it)->GetName()));
        }
      }
    }
  }

  void
  RootOutputTree::fillTree() const {
    fillTTree(metaTree_, metaBranches_);
    bool saveMemory = (saveMemoryObjectThreshold_ > -1);
    fillTTree(tree_, producedBranches_, saveMemory);
    if (currentlyFastCloning_) {
      fillTTree(tree_, unclonedReadBranches_, saveMemory);
    } else {
      fillTTree(tree_, readBranches_, saveMemory);
    }
  }

  void
  RootOutputTree::addBranch(BranchDescription const& prod,
                            void const*& pProd) {
    bool produced = prod.produced();
      TBranch *branch = tree_->Branch(prod.branchName().c_str(),
                 prod.wrappedCintName().c_str(),
                 &pProd,
                 (prod.basketSize() == BranchDescription::invalidBasketSize ? basketSize_ : prod.basketSize()),
                 (prod.splitLevel() == BranchDescription::invalidSplitLevel ? splitLevel_ : prod.splitLevel()));
      if (prod.compression() != BranchDescription::invalidCompression) {
         branch->SetCompressionSettings(prod.compression());
         //branch->SetCompressionAlgorithm(prod.compress() / 100);
         //branch->SetCompressionLevel(prod.compress() % 100);
      }
      if (produced) {
        producedBranches_.push_back(branch);
      } else {
        readBranches_.push_back(branch);
      }
  }

}  // art
