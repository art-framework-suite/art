#ifndef art_Framework_Modules_detail_SamplingDelayedReader_h
#define art_Framework_Modules_detail_SamplingDelayedReader_h
// vim: set sw=2:

#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

#include <map>
#include <memory>
#include <string>

extern "C" {
#include "sqlite3.h"
}

class TFile;
class TTree;

namespace art {
  namespace detail {

    class SamplingDelayedReader final : public DelayedReader {
    public:
      ~SamplingDelayedReader() = default;

      SamplingDelayedReader(SamplingDelayedReader const&) = delete;
      SamplingDelayedReader& operator=(SamplingDelayedReader const&) = delete;

      SamplingDelayedReader(FileFormatVersion,
                            sqlite3* db,
                            std::vector<input::EntryNumber> const& entrySet,
                            input::BranchMap const&,
                            cet::exempt_ptr<TTree> tree,
                            int64_t saveMemoryObjectThreshold,
                            cet::exempt_ptr<BranchIDLists const> branchIDLists,
                            BranchType branchType,
                            EventID const& id,
                            bool compactSubRunRanges);

      std::unique_ptr<EDProduct> getProduct(BranchKey const&,
                                            std::string const& wrappedType,
                                            RangeSet&) const;

    private:
      std::unique_ptr<EDProduct> getProduct_(BranchKey const&,
                                             TypeID const&,
                                             RangeSet&) const override;
      void setGroupFinder_(
        cet::exempt_ptr<EDProductGetterFinder const>) override;

      FileFormatVersion fileFormatVersion_;
      sqlite3* db_;
      std::vector<input::EntryNumber> const entrySet_;
      input::BranchMap const& branches_;
      cet::exempt_ptr<TTree> tree_;
      int64_t saveMemoryObjectThreshold_;
      cet::exempt_ptr<EDProductGetterFinder const> groupFinder_;
      cet::exempt_ptr<BranchIDLists const>
        branchIDLists_; // Only for backwards compatibility
      BranchType branchType_;
      EventID eventID_;
      bool const compactSubRunRanges_;
    };

  } // namespace detail
} // namespace art

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Modules_detail_SamplingDelayedReader_h */
