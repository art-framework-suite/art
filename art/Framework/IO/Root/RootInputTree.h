#ifndef art_Framework_IO_Root_RootInputTree_h
#define art_Framework_IO_Root_RootInputTree_h
// vim: set sw=2:

//
//  RootInputTree
//
//  Used by ROOT input sources.
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/IO/Root/BranchMapperWithReader.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/IO/Root/detail/rangeSetFromFileIndex.h"
#include "art/Framework/IO/Root/detail/resolveRangeSet.h"
#include "art/Framework/Principal/ClosedRangeSetHandler.h"
#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "cetlib/exempt_ptr.h"

#include "TBranch.h"
#include "TTree.h"

#include <memory>
#include <string>
#include <vector>

class TFile;
struct sqlite3;

namespace art {

  namespace detail {

    template <typename AUX>
    void
    mergeAuxiliary(AUX& left, AUX const& right)
    {
      left.mergeAuxiliary(right);
    }

    template <>
    inline void
    mergeAuxiliary(EventAuxiliary&, EventAuxiliary const&)
    {}

    template <BranchType, typename ID>
    RangeSet makeFullRangeSet(ID const&);

    template <>
    inline RangeSet
    makeFullRangeSet<InSubRun, SubRunID>(SubRunID const& id)
    {
      return RangeSet::forSubRun(id);
    }

    template <>
    inline RangeSet
    makeFullRangeSet<InRun, RunID>(RunID const& id)
    {
      return RangeSet::forRun(id);
    }
  }

  class DelayedReader;
  class Principal;

  class RootInputTree {
  public:
    using BranchMap = input::BranchMap;
    using EntryNumber = input::EntryNumber;
    using EntryNumbers = input::EntryNumbers;

    RootInputTree(cet::exempt_ptr<TFile>,
                  BranchType,
                  int64_t saveMemoryObjectThreshold,
                  cet::exempt_ptr<RootInputFile>,
                  bool compactSubRunRanges = false,
                  bool missingOK = false,
                  bool rangesEnabled = true);
    RootInputTree(RootInputTree const&) = delete;
    RootInputTree& operator=(RootInputTree const&) = delete;

    explicit operator bool() const { return isValid(); }

    bool isValid() const;
    bool hasBranch(std::string const& branchName) const;
    void addBranch(BranchKey const&, BranchDescription const&);
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
      return std::all_of(numbers.cbegin(), numbers.cend(), [this](auto entry) {
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

    void
    fillGroups(Principal& p)
    {
      if ((metaTree_ == nullptr) || (metaTree_->GetNbranches() == 0)) {
        return;
      }
      // Loop over provenance
      for (auto const& b : branches_) {
        p.fillGroup(b.second.branchDescription_);
      }
    }

    // The BranchIDLists are passed so we can configure the
    // ProductIDStreamer, which is necessary for backwards
    // compatibility with ProductIDs from older files.

    // FIXME: Since in older files ProductIDs (and therefore the
    // BranchIDLists) were only meaningfully used for events, we
    // should not need to worry about passing the BranchIDLists in the
    // SQLite-based makeDelayedReader overload used for (Sub)Runs.

    std::unique_ptr<DelayedReader> makeDelayedReader(
      FileFormatVersion,
      cet::exempt_ptr<BranchIDLists const> branchIDLists,
      BranchType,
      std::vector<EntryNumber> const& entrySet,
      EventID);

    std::unique_ptr<DelayedReader> makeDelayedReader(
      FileFormatVersion,
      sqlite3* inputDB,
      cet::exempt_ptr<BranchIDLists const> branchIDLists,
      BranchType,
      std::vector<EntryNumber> const& entrySet,
      EventID);

    std::unique_ptr<BranchMapper> makeBranchMapper() const;

    template <typename AUX>
    AUX
    getAux(EntryNumber const entry)
    {
      auto aux = std::make_unique<AUX>();
      auto pAux = aux.get();
      auxBranch_->SetAddress(&pAux);
      setEntryNumber(entry);
      input::getEntry(auxBranch_, entry);
      auxBranch_->ResetAddress();
      return *aux;
    }

    template <typename AUX>
    std::unique_ptr<RangeSetHandler>
    fillAux(FileFormatVersion const fileFormatVersion,
            EntryNumbers const& entries,
            FileIndex const& fileIndex,
            sqlite3* db,
            std::string const& filename,
            AUX& aux)
    {
      auto auxResult = getAux<AUX>(entries[0]);
      if ((fileFormatVersion.value_ < 9) || (db == nullptr)) {
        auxResult.setRangeSetID(-1u);
        auto const& rs = detail::rangeSetFromFileIndex(
          fileIndex, auxResult.id(), compactSubRunRanges_);
        std::swap(aux, auxResult);
        return std::make_unique<ClosedRangeSetHandler>(rs);
      }

      auto resolve_info = [db, &filename](auto const id,
                                          bool const compactSubRunRanges) {
        return detail::resolveRangeSetInfo(
          db, filename, AUX::branch_type, id, compactSubRunRanges);
      };

      auto rangeSetInfo =
        resolve_info(auxResult.rangeSetID(), compactSubRunRanges_);
      for (auto i = entries.cbegin() + 1, e = entries.cend(); i != e; ++i) {
        auto const& tmpAux = getAux<AUX>(*i);
        detail::mergeAuxiliary(auxResult, tmpAux);
        rangeSetInfo.update(
          resolve_info(tmpAux.rangeSetID(), compactSubRunRanges_),
          compactSubRunRanges_);
      }

      auxResult.setRangeSetID(-1u); // Range set of new auxiliary is invalid
      std::swap(aux, auxResult);
      return std::make_unique<ClosedRangeSetHandler>(
        resolveRangeSet(rangeSetInfo));
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

    TBranch*
    productProvenanceBranch() const
    {
      return productProvenanceBranch_;
    }

  private:
    cet::exempt_ptr<TFile> filePtr_;
    // We use bare pointers for pointers to some ROOT entities.
    // Root owns them and uses bare pointers internally,
    // therefore, using smart pointers here will do no good.
    TTree* tree_{nullptr};
    TTree* metaTree_{nullptr};
    BranchType branchType_;
    int64_t const saveMemoryObjectThreshold_;
    TBranch* auxBranch_{nullptr};
    TBranch* productProvenanceBranch_{nullptr};
    EntryNumber entries_{0};
    EntryNumber entryNumber_{-1};
    BranchMap branches_{};
    cet::exempt_ptr<RootInputFile> primaryFile_;
    bool const compactSubRunRanges_;
    bool const rangesEnabled_{true};
  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootInputTree_h */
