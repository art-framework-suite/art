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

namespace art {
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
                                 BranchDescription const& sampledEventInfoDesc,
                                 std::map<BranchKey, BranchDescription>&
                                   oldKeyToSampledProductDescription,
                                 ModuleDescription const& moduleDescription,
                                 bool readIncomingParameterSets,
                                 MasterProductRegistry& mpr);

      EventID nextEvent() const;
      bool readyForNextEvent();

      EntriesForID_t treeEntries(BranchType);
      Products_t productsFor(EntriesForID_t const& entries, BranchType);

      std::unique_ptr<EventPrincipal> readEvent(EventID const& eventID,
                                                ProcessConfiguration const& pc);

    private:
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
      ProductRegistry productListHolder_{};
      ProductTable presentEventProducts_{};
      std::unique_ptr<BranchIDLists> branchIDLists_{
        nullptr}; // Only used for maintaining backwards compatibility
    };
  }
}

#endif /* art_Framework_Modules_detail_SamplingInputFile_h */

// Local Variables:
// mode: c++
// End:
