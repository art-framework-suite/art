#ifndef art_Framework_IO_Root_RootDelayedReader_h
#define art_Framework_IO_Root_RootDelayedReader_h
// vim: set sw=2:

#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/IO/Root/RootInputFile.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchType.h"

#include <map>
#include <memory>
#include <string>

class TFile;

namespace art {

class RootDelayedReader final : public DelayedReader {

public: // MEMBER FUNCTIONS

  virtual
  ~RootDelayedReader();

  RootDelayedReader(RootDelayedReader const&) = delete;

  RootDelayedReader&
  operator=(RootDelayedReader const&) = delete;

  RootDelayedReader(input::EntryNumber const&,
                    std::shared_ptr<input::BranchMap const>,
                    std::shared_ptr<TFile const>,
                    int64_t saveMemoryObjectThreshold,
                    cet::exempt_ptr<RootInputFile> primaryFile,
                    BranchType branchType, EventID);

private: // MEMBER FUNCTIONS

  virtual
  std::unique_ptr<EDProduct>
  getProduct_(BranchKey const&, TypeID const&) const override;

  virtual
  void
  setGroupFinder_(cet::exempt_ptr<EDProductGetterFinder const>) override;

  virtual
  void
  mergeReaders_(std::shared_ptr<DelayedReader>) override;

  virtual
  int
  openNextSecondaryFile_(int idx) override;

private: // MEMBER DATA

  input::EntryNumber const entryNumber_;
  std::shared_ptr<input::BranchMap const> branches_;
  // NOTE: filePtr_ appears to be unused, but is needed to prevent
  // the TFile containing the branch from being reclaimed.
  std::shared_ptr<TFile const> filePtr_;
  int64_t saveMemoryObjectThreshold_;
  std::shared_ptr<DelayedReader> nextReader_;
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
