#ifndef art_Framework_IO_Root_RootTree_h
#define art_Framework_IO_Root_RootTree_h
// vim: set sw=2:

//
//  RootTree
//
//  Used by ROOT input sources.
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/IO/Root/BranchMapperWithReader.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "TBranch.h"
#include "TTree.h"
#include "cpp0x/memory"
#include <string>
#include <vector>

class TFile;

namespace art {

class DelayedReader;
class Principal;

class RootTree {
public:
  typedef input::BranchMap BranchMap;
  typedef input::EntryNumber EntryNumber;
public:
  RootTree(std::shared_ptr<TFile>, BranchType const&,
           int64_t saveMemoryObjectThreshold, cet::exempt_ptr<RootInputFile>);
  RootTree(RootTree const&) = delete;
  RootTree& operator=(RootTree const&) = delete;

  bool isValid() const;
  bool hasBranch(std::string const& branchName) const;
  void addBranch(BranchKey const&, BranchDescription const&,
                 std::string const& branchName,
                 bool const presentInSomeFile);
  void dropBranch(std::string const& branchName);

  bool
  next()
  {
    return ++entryNumber_ < entries_;
  }

  bool
  previous()
  {
    return --entryNumber_ >= 0;
  }

  bool
  current()
  {
    return (entryNumber_ < entries_) && (entryNumber_ >= 0);
  }

  void
  rewind()
  {
    entryNumber_ = 0;
  }

  EntryNumber const&
  entryNumber() const
  {
    return entryNumber_;
  }

  EntryNumber const&
  entries() const
  {
    return entries_;
  }

  void setEntryNumber(EntryNumber theEntryNumber);

  std::vector<std::string> const&
  branchNames() const
  {
    return branchNames_;
  }

  void
  fillGroups(Principal& item)
  {
    if ((metaTree_ == 0) || (metaTree_->GetNbranches() == 0)) {
      return;
    }
    // Loop over provenance
    for (auto I = branches_->cbegin(), E = branches_->cend(); I != E; ++I) {
      item.addGroup(I->second.branchDescription_);
    }
  }

  std::unique_ptr<DelayedReader>
  makeDelayedReader(BranchType, EventID) const;

  std::unique_ptr<BranchMapper>
  makeBranchMapper() const;

  template<typename T>
  void
  fillAux(T*& pAux) const
  {
    auxBranch_->SetAddress(&pAux);
    input::getEntry(auxBranch_, entryNumber_);
  }

  TTree const*
  tree() const
  {
    return tree_;
  }

  TTree const*
  metaTree() const
  {
    return metaTree_;
  }

  void setCacheSize(unsigned int cacheSize) const;

  void setTreeMaxVirtualSize(int treeMaxVirtualSize);

  BranchMap const&
  branches() const
  {
    return *branches_;
  }

  TBranch*
  productProvenanceBranch() const
  {
    return productProvenanceBranch_;
  }

private:
  std::shared_ptr<TFile> filePtr_;
  // We use bare pointers for pointers to some ROOT entities.
  // Root owns them and uses bare pointers internally,
  // therefore, using smart pointers here will do no good.
  TTree* tree_;
  TTree* metaTree_;
  BranchType branchType_;
  int64_t const saveMemoryObjectThreshold_;
  TBranch* auxBranch_;
  TBranch* productProvenanceBranch_;
  EntryNumber entries_;
  EntryNumber entryNumber_;
  std::vector<std::string> branchNames_;
  std::shared_ptr<BranchMap> branches_;
  cet::exempt_ptr<RootInputFile> primaryFile_;
};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootTree_h */
