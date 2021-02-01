#ifndef art_Framework_IO_Root_RootDelayedReader_h
#define art_Framework_IO_Root_RootDelayedReader_h
// vim: set sw=2:

#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

extern "C" {
#include "sqlite3.h"
}

class TFile;

namespace art {

  using secondary_opener_t =
    std::function<int(int, BranchType, EventID const&)>;

  class RootInputTree;

  class RootDelayedReader final : public DelayedReader {
  public:
    ~RootDelayedReader() = default;
    RootDelayedReader(RootDelayedReader const&) = delete;
    RootDelayedReader& operator=(RootDelayedReader const&) = delete;

    RootDelayedReader(FileFormatVersion,
                      sqlite3* db,
                      std::vector<input::EntryNumber> const& entrySet,
                      input::BranchMap const&,
                      cet::exempt_ptr<RootInputTree> tree,
                      int64_t saveMemoryObjectThreshold,
                      secondary_opener_t secondaryFileOpener,
                      cet::exempt_ptr<BranchIDLists const> branchIDLists,
                      BranchType branchType,
                      EventID,
                      bool compactSubRunRanges);

  private: // MEMBER FUNCTIONS
    std::unique_ptr<EDProduct> getProduct_(BranchKey const&,
                                           TypeID const&,
                                           RangeSet&) const override;
    void setGroupFinder_(cet::exempt_ptr<EDProductGetterFinder const>) override;
    int openNextSecondaryFile_(int idx) override;

  private: // MEMBER DATA
    FileFormatVersion fileFormatVersion_;
    sqlite3* db_;
    std::vector<input::EntryNumber> const entrySet_;
    input::BranchMap const& branches_;
    cet::exempt_ptr<RootInputTree> tree_;
    int64_t saveMemoryObjectThreshold_;
    cet::exempt_ptr<EDProductGetterFinder const> groupFinder_;
    secondary_opener_t openSecondaryFile_;
    cet::exempt_ptr<BranchIDLists const>
      branchIDLists_; // Only for backwards compatibility
    BranchType branchType_;
    EventID eventID_;
    bool const compactSubRunRanges_;
  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootDelayedReader_h */
