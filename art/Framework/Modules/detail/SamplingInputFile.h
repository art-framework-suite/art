#ifndef art_Framework_Modules_detail_SamplingInputFile_h
#define art_Framework_Modules_detail_SamplingInputFile_h

#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProductRegistry.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SampledInfo.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/sqlite/Connection.h"

#include "TFile.h"
#include "TTree.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace art {
  class BranchChildren;
  class GroupSelectorRules;
  class MasterProductRegistry;
  class ModuleDescription;
  struct ProcessConfiguration;

  namespace detail {

    class SamplingInputFile {
    public:
      using EntriesForID_t = std::map<EventID, input::EntryNumbers>;
      using Products_t =
        std::map<BranchKey, std::vector<std::unique_ptr<EDProduct>>>;

      explicit SamplingInputFile(std::string const& dataset,
                                 std::string const& filename,
                                 double weight,
                                 double probability,
                                 EventID const& firstEvent,
                                 GroupSelectorRules const& groupSelectorRules,
                                 bool dropDescendants,
                                 unsigned int treeCacheSize,
                                 int64_t treeMaxVirtualSize,
                                 int64_t saveMemoryObjectThreshold,
                                 BranchDescription const& sampledEventInfoDesc,
                                 bool compactRangeSets,
                                 std::map<BranchKey, BranchDescription>&
                                   oldKeyToSampledProductDescription,
                                 ModuleDescription const& moduleDescription,
                                 bool readIncomingParameterSets,
                                 MasterProductRegistry& mpr);

      EventID nextEvent() const;
      bool readyForNextEvent();

      EntriesForID_t treeEntries(BranchType);
      Products_t productsFor(EntriesForID_t const& entries, BranchType);

      template <typename T>
      SampledInfo<T> sampledInfoFor(EntriesForID_t const& entries);

      std::unique_ptr<EventPrincipal> readEvent(
        EventID const& eventID,
        ProcessConfigurations const& sampled_pcs,
        ProcessConfiguration const& current_pc);

    private:
      void dropOnInput_(GroupSelectorRules const& rules,
                        BranchChildren const& children,
                        bool dropDescendants,
                        ProductList& productList);
      bool updateEventEntry_(FileIndex::const_iterator& iter,
                             input::EntryNumber& entry) const;
      TTree* treeForBranchType_(BranchType bt) const;
      EventAuxiliary auxiliaryForEntry_(input::EntryNumber entry);
      History historyForEntry_(input::EntryNumber entry);

      std::string const dataset_;
      std::unique_ptr<TFile> file_;
      double const weight_;
      double const probability_;
      EventID const firstEvent_;
      cet::sqlite::Connection sqliteDB_{}; // Start with invalid connection.
      int64_t saveMemoryObjectThreshold_;
      FileIndex fileIndex_{};
      FileFormatVersion fileFormatVersion_{};
      FileIndex::const_iterator fiIter_{fileIndex_.cbegin()};
      FileIndex::const_iterator fiEnd_{fileIndex_.cend()};
      input::EntryNumber currentEventEntry_{-1};
      TTree* runTree_{nullptr};
      TTree* subRunTree_{nullptr};
      TTree* eventTree_{nullptr};
      TTree* eventMetaTree_{nullptr};
      TBranch* auxBranch_{nullptr};
      TBranch* productProvenanceBranch_{nullptr};
      TTree* eventHistoryTree_{nullptr};
      input::BranchMap branches_{};
      BranchDescription const& sampledEventInfoDesc_;
      bool compactRangeSets_;
      ProductRegistry productListHolder_{};
      ProductTable presentEventProducts_{};
      std::unique_ptr<BranchIDLists> branchIDLists_{
        nullptr}; // Only used for maintaining backwards compatibility
    };

    template <typename T>
    T id_for(EventID const& id);

    template <>
    inline RunID
    id_for<RunID>(EventID const& id)
    {
      return id.runID();
    }

    template <>
    inline SubRunID
    id_for<SubRunID>(EventID const& id)
    {
      return id.subRunID();
    }

    template <typename T>
    SampledInfo<T>
    SamplingInputFile::sampledInfoFor(EntriesForID_t const& entries)
    {
      // We use a set because it is okay for multiple entries of the
      // same (sub)run to be present in the FileIndex--these correspond
      // to (sub)run fragments.  However, we do not want these to appear
      // as separate entries in the SampledInfo object.
      std::set<T> ids;
      for (auto const& pr : entries) {
        auto const& invalid_event_id = pr.first;
        ids.insert(id_for<T>(invalid_event_id));
      }

      return SampledInfo<T>{
        weight_, probability_, std::vector<T>(cbegin(ids), cend(ids))};
    }

  }
}

#endif /* art_Framework_Modules_detail_SamplingInputFile_h */

// Local Variables:
// mode: c++
// End:
