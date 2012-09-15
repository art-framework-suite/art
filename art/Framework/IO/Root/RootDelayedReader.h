#ifndef art_Framework_IO_Root_RootDelayedReader_h
#define art_Framework_IO_Root_RootDelayedReader_h

// ======================================================================
//
// RootDelayedReader - used by ROOT input sources;
//                     pretends to support file reading.
//
// ======================================================================

#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "cpp0x/memory"
#include <map>
#include <string>

// ----------------------------------------------------------------------

class TFile;

namespace art {
  class RootDelayedReader;
}
class art::RootDelayedReader :
  public art::DelayedReader {
public:
  RootDelayedReader(RootDelayedReader const&) = delete;
  RootDelayedReader& operator=(RootDelayedReader const&) = delete;

  typedef input::BranchMap BranchMap;
  typedef input::EntryNumber EntryNumber;
  typedef input::BranchInfo BranchInfo;
  typedef input::BranchMap::const_iterator iterator;

  RootDelayedReader(EntryNumber const& entry,
                    std::shared_ptr<BranchMap const> bMap,
                    std::shared_ptr<TFile const> filePtr,
                    bool oldFormat);

  virtual ~RootDelayedReader();

private:
  virtual std::unique_ptr<EDProduct> getProduct_(BranchKey const& k, art::TypeID const &wrapper_type) const;
  virtual void setGroupFinder_(cet::exempt_ptr<EventPrincipal const>);
  virtual void mergeReaders_(std::shared_ptr<DelayedReader> other) {nextReader_ = other;}
  BranchMap const& branches() const {return *branches_;}
  iterator branchIter(BranchKey const& k) const {return branches().find(k);}
  bool found(iterator const& iter) const {return iter != branches().end();}
  BranchInfo const& getBranchInfo(iterator const& iter) const {return iter->second; }

  EntryNumber const entryNumber_;
  std::shared_ptr<BranchMap const> branches_;
  // NOTE: filePtr_ appears to be unused, but is needed to prevent
  // the TFile containing the branch from being reclaimed.
  std::shared_ptr<TFile const> filePtr_;
  std::shared_ptr<DelayedReader> nextReader_;
  bool customStreamers_;
  bool oldFormat_;

  cet::exempt_ptr<EventPrincipal const> groupFinder_;

}; // RootDelayedReader

#endif /* art_Framework_IO_Root_RootDelayedReader_h */

// Local Variables:
// mode: c++
// End:
