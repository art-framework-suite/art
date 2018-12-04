#ifndef art_Framework_Modules_detail_SamplingInputFile_h
#define art_Framework_Modules_detail_SamplingInputFile_h

#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/Modules/SampledInfo.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProductRegistry.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/sqlite/Connection.h"

#include "TFile.h"
#include "TTree.h"

#include <memory>
#include <string>

namespace art {
  class MasterProductRegistry;
  struct ProcessConfiguration;

  namespace detail {
    class SamplingInputFile {
    public:
      explicit SamplingInputFile(
        std::string const& dataset,
        std::string const& filename,
        double weight,
        double probability,
        ProductDescriptions const& sampledInfoDescriptions,
        bool readIncomingParameterSets,
        MasterProductRegistry& mpr);
      bool entryForNextEvent(input::EntryNumber& entry);
      SampledInfo<RunID> fillRun(RunPrincipal& rp) const;
      SampledInfo<SubRunID> fillSubRun(SubRunPrincipal& srp) const;
      std::unique_ptr<EventPrincipal> readEvent(input::EntryNumber entry,
                                                EventID const& eventID,
                                                ProcessConfiguration const& pc);

    private:
      EventAuxiliary auxiliaryForEntry_(input::EntryNumber);
      History historyForEntry_(input::EntryNumber);

      std::string const dataset_;
      std::unique_ptr<TFile> file_;
      double const weight_;
      double const probability_;
      cet::sqlite::Connection sqliteDB_{}; // Start with invalid connection.
      FileIndex fileIndex_{};
      FileFormatVersion fileFormatVersion_{};
      FileIndex::const_iterator fiIter_{fileIndex_.cbegin()};
      FileIndex::const_iterator fiEnd_{fileIndex_.cend()};
      TTree* eventTree_{nullptr};
      TTree* eventMetaTree_{nullptr};
      TBranch* auxBranch_{nullptr};
      TBranch* productProvenanceBranch_{nullptr};
      TTree* eventHistoryTree_{nullptr};
      input::BranchMap branches_{};
      ProductDescriptions const& sampledInfoDescriptions_;
      ProductRegistry productListHolder_{};
      ProductTables presentProducts_{ProductTables::invalid()};
      std::unique_ptr<BranchIDLists> branchIDLists_{
        nullptr}; // Only used for maintaining backwards compatibility
    };
  }
}

#endif /* art_Framework_Modules_detail_SamplingInputFile_h */

// Local Variables:
// mode: c++
// End:
