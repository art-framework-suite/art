#ifndef art_Framework_IO_Root_RootDelayedReader_h
#define art_Framework_IO_Root_RootDelayedReader_h
// vim: set sw=2:

#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "canvas/Persistency/Provenance/BranchID.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

#include <map>
#include <memory>
#include <string>

extern "C" {
#include "sqlite3.h"
}

class TFile;

namespace art {

  class RootInputFile;
  class RootTree;

  class RootDelayedReader final : public DelayedReader {

  public: // MEMBER FUNCTIONS

    ~RootDelayedReader() = default;

    RootDelayedReader(RootDelayedReader const&) = delete;

    RootDelayedReader& operator=(RootDelayedReader const&) = delete;

    RootDelayedReader(sqlite3* db,
                      std::vector<input::EntryNumber> const& entrySet,
                      std::shared_ptr<input::BranchMap const>,
                      cet::exempt_ptr<RootTree> tree,
                      int64_t saveMemoryObjectThreshold,
                      cet::exempt_ptr<RootInputFile> primaryFile,
                      BranchType branchType, EventID);


  private: // MEMBER FUNCTIONS

    std::unique_ptr<EDProduct> getProduct_(BranchKey const&, TypeID const&) const override;
    RangeSet const& getRangeSet_(BranchID const&) const override;
    void setGroupFinder_(cet::exempt_ptr<EDProductGetterFinder const>) override;
    int openNextSecondaryFile_(int idx) override;

  private: // MEMBER DATA

    using RS_checksum_t = unsigned;

    sqlite3* db_;
    std::vector<input::EntryNumber> const entrySet_;
    mutable std::map<RS_checksum_t, RangeSet> rangeSets_;
    mutable std::map<BranchID,RS_checksum_t> productRangeSetChecksums_;
    std::shared_ptr<input::BranchMap const> branches_;
    cet::exempt_ptr<RootTree> tree_;
    int64_t saveMemoryObjectThreshold_;
    cet::exempt_ptr<EDProductGetterFinder const> groupFinder_;
    cet::exempt_ptr<RootInputFile> primaryFile_;
    BranchType branchType_;
    EventID eventID_;

  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootDelayedReader_h */
