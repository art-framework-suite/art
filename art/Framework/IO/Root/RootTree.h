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
#include "art/Framework/IO/Root/detail/getFileContributors.h"
#include "art/Framework/Principal/BoundedRangeSetHandler.h"
#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"

#include "TBranch.h"
#include "TTree.h"

#include <memory>
#include <string>
#include <vector>

class TFile;
class sqlite3;

namespace art {

  namespace detail {

    template <typename AUX>
    void mergeAuxiliary(AUX& left, AUX const& right)
    {
      left.mergeAuxiliary(right);
    }

    template <>
    inline void mergeAuxiliary(EventAuxiliary&,
                               EventAuxiliary const&)
    {}
  }

  class DelayedReader;
  class Principal;

  class RootTree {
  public:
    using BranchMap = input::BranchMap;
    using EntryNumber = input::EntryNumber;
    using EntryNumbers = input::EntryNumbers;
  public:
    RootTree(std::shared_ptr<TFile>,
             BranchType const&,
             int64_t saveMemoryObjectThreshold,
             cet::exempt_ptr<RootInputFile>,
             bool missingOK = false);
    RootTree(RootTree const&) = delete;
    RootTree& operator=(RootTree const&) = delete;

    explicit operator bool () const { return isValid(); }

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
    current(EntryNumbers const& numbers)
    {
      assert(!numbers.empty());
      return std::all_of(numbers.cbegin(),
                         numbers.cend(),
                         [this](auto entry){
                           return (entry < entries_) && (entry >= 0);
                         });
    }

    void
    rewind()
    {
      entryNumber_ = 0;
    }

    EntryNumber
    entryNumber() const
    {
      return entryNumber_;
    }

    EntryNumber
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
      for (auto const& b : *branches_) {
        item.addGroup(b.second.branchDescription_);
      }
    }

    std::unique_ptr<DelayedReader>
    makeDelayedReader(BranchType,
                      std::vector<EntryNumber> const& entrySet,
                      EventID);

    std::unique_ptr<DelayedReader>
    makeDelayedReader(sqlite3* inputDB,
                      BranchType,
                      std::vector<EntryNumber> const& entrySet,
                      EventID);

    std::unique_ptr<BranchMapper>
    makeBranchMapper() const;

    template<typename AUX>
    AUX getAux(EntryNumber const entry)
    {
      auto aux  = std::make_unique<AUX>();
      auto pAux = aux.get();
      auxBranch_->SetAddress(&pAux);
      setEntryNumber(entry);
      input::getEntry(auxBranch_, entry);
      return *aux;
    }

    template<typename AUX>
    std::unique_ptr<BoundedRangeSetHandler> fillAux(EntryNumbers const& entries,
                                                    sqlite3* db,
                                                    std::string const& filename,
                                                    AUX& aux)
    {
      auto auxResult = getAux<AUX>(entries[0]);
      auto rangeSet = detail::getContributors(db,
                                              filename,
                                              AUX::branch_type,
                                              auxResult.rangeSetID());
      for(auto i = entries.cbegin()+1, e = entries.cend(); i!=e; ++i) {
        auto const& tmpAux = getAux<AUX>(*i);
        detail::mergeAuxiliary(auxResult, tmpAux);
        auto const& tmpRangeSet = detail::getContributors(db,
                                                          filename,
                                                          AUX::branch_type,
                                                          tmpAux.rangeSetID());
        rangeSet.merge(tmpRangeSet);
      }
      auto merged = std::make_unique<BoundedRangeSetHandler>(rangeSet);
      auxResult.setRangeSetID(-1u); // Range set of new auxiliary is invalid
      std::swap(aux, auxResult);
      return merged;
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
    TTree* tree_ {nullptr};
    TTree* metaTree_ {nullptr};
    BranchType branchType_;
    int64_t const saveMemoryObjectThreshold_;
    TBranch* auxBranch_ {nullptr};
    TBranch* productProvenanceBranch_ {nullptr};
    EntryNumber entries_ {0};
    EntryNumber entryNumber_ {-1};
    std::vector<std::string> branchNames_ {};
    std::shared_ptr<BranchMap> branches_ {std::make_shared<BranchMap>()};
    cet::exempt_ptr<RootInputFile> primaryFile_;
  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootTree_h */
