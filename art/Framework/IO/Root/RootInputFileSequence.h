#ifndef art_Framework_IO_Root_RootInputFileSequence_h
#define art_Framework_IO_Root_RootInputFileSequence_h
// vim: set sw=2:

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/GroupSelectorRules.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/IO/Root/DuplicateChecker.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/OptionalSequence.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"

#include <memory>
#include <string>
#include <vector>

namespace art {

class DuplicateChecker;
class FileCatalogItem;
class FileIndex;
class InputFileCatalog;
class MasterProductRegistry;
class RootInputFile;

class RootInputFileSequence {

public: // TYPES

  using RootInputFileSharedPtr = std::shared_ptr<RootInputFile>;
  using EntryNumber = input::EntryNumber;

public: // MEMBER FUNCTIONS

  RootInputFileSequence(RootInputFileSequence const&) = delete;

  RootInputFileSequence&
  operator=(RootInputFileSequence const&) = delete;

  struct Config {

    using Name = fhicl::Name;
    template <typename T> using Atom = fhicl::Atom<T>;
    template <typename T> using OptionalAtom     = fhicl::OptionalAtom<T>;
    template <typename T> using OptionalSequence = fhicl::OptionalSequence<T>;
    template <typename T> using Sequence         = fhicl::Sequence<T>;
    template <typename T> using Table = fhicl::Table<T>;
    template <typename T> using TableFragment    = fhicl::TableFragment<T>;

    TableFragment<DuplicateChecker::Config> dc;
    Atom<EventNumber_t> skipEvents { Name("skipEvents"), 0 };
    Atom<bool> noEventSort { Name("noEventSort"), false };
    Atom<bool> skipBadFiles { Name("skipBadFiles"), false };
    Atom<unsigned> cacheSize { Name("cacheSize"), 0u };
    Atom<std::int64_t> treeMaxVirtualSize { Name("treeMaxVirtualSize"), -1 };
    Atom<std::int64_t> saveMemoryObjectThreshold { Name("saveMemoryObjectThreshold"), -1 };
    Atom<bool> delayedReadEventProducts { Name("delayedReadEventProducts"), true };
    Atom<bool> delayedReadSubRunProducts { Name("delayedReadSubRunProducts"), false };
    Atom<bool> delayedReadRunProducts { Name("delayedReadRunProducts"), false };
    Sequence<std::string> inputCommands { Name("inputCommands"), std::vector<std::string>{"keep *"} };
    Atom<bool> dropDescendantsOfDroppedBranches { Name("dropDescendantsOfDroppedBranches"), true };
    Atom<bool> readParameterSets { Name("readParameterSets"), true };

    struct SecondaryFile {
      Atom<std::string> a { Name("a"), "" };
      Sequence<std::string> b { Name("b"), std::vector<std::string>{} };
    };

    OptionalSequence< Table<SecondaryFile> > secondaryFileNames { Name("secondaryFileNames") };
    OptionalAtom<RunNumber_t>    hasFirstRun { Name("firstRun") };
    OptionalAtom<SubRunNumber_t> hasFirstSubRun { Name("firstSubRun") };
    OptionalAtom<EventNumber_t>  hasFirstEvent { Name("firstEvent") };
    OptionalAtom<RunNumber_t> setRunNumber { Name("setRunNumber") };

  };

  RootInputFileSequence(fhicl::TableFragment<Config> const&,
                        InputFileCatalog&,
                        FastCloningInfoProvider const&,
                        InputSource::ProcessingMode,
                        MasterProductRegistry&,
                        ProcessConfiguration const&);
  void
  endJob();

  std::unique_ptr<FileBlock>
  readFile_();

  std::unique_ptr<RootInputFile>
  openSecondaryFile(std::string const& name,
                    cet::exempt_ptr<RootInputFile> primaryFile);

  void
  closeFile_();

  void
  skip(int offset);

  void
  rewind_();

  EventID
  seekToEvent(EventID const&, bool exact = false);

  EventID
  seekToEvent(off_t offset, bool exact = false);

  input::ItemType
  getNextItemType();

  std::unique_ptr<RunPrincipal>
  readIt(RunID const&);

  std::unique_ptr<RunPrincipal>
  readRun_();

  std::unique_ptr<SubRunPrincipal>
  readIt(SubRunID const&, cet::exempt_ptr<RunPrincipal>);

  std::unique_ptr<SubRunPrincipal>
  readSubRun_(cet::exempt_ptr<RunPrincipal>);

  std::unique_ptr<EventPrincipal>
  readIt(EventID const&, bool exact = false);

  std::unique_ptr<EventPrincipal>
  readEvent_();

  RootInputFileSharedPtr
  rootFileForLastReadEvent() const
  {
    return rootFileForLastReadEvent_;
  }

  RootInputFileSharedPtr
  rootFile() const
  {
    return rootFile_;
  }

  std::unique_ptr<RangeSetHandler> runRangeSetHandler();
  std::unique_ptr<RangeSetHandler> subRunRangeSetHandler();

  std::vector<std::vector<std::string>> const&
  secondaryFileNames() const
  {
    return secondaryFileNames_;
  }

  EventID
  origEventID() const
  {
    return origEventID_;
  }

  EventNumber_t
  eventsToSkip() const
  {
    return eventsToSkip_;
  }

  FastCloningInfoProvider const&
  fastCloningInfo() const
  {
    return  fastCloningInfo_;
  }

  unsigned int
  treeCacheSize() const
  {
    return treeCacheSize_;
  }

  int64_t
  treeMaxVirtualSize() const
  {
    return treeMaxVirtualSize_;
  }

  int64_t
  saveMemoryObjectThreshold() const
  {
    return saveMemoryObjectThreshold_;
  };

  bool
  delayedReadEventProducts() const
  {
    return delayedReadEventProducts_;
  }

  bool
  delayedReadSubRunProducts() const
  {
    return delayedReadSubRunProducts_;
  }

  bool
  delayedReadRunProducts() const
  {
    return delayedReadRunProducts_;
  }

  InputSource::ProcessingMode const&
  processingMode()
  {
    return  processingMode_;
  }

  void finish();

private: // MEMBER FUNCTIONS

  void
  initFile(bool skipBadFiles);

  bool
  nextFile();

  bool
  previousFile();

  void
  rewindFile();

  std::vector<FileCatalogItem> const&
  fileCatalogItems() const;

  ProcessConfiguration const&
  processConfiguration() const;

  bool
  primary() const;

private: // MEMBER DATA

  InputFileCatalog& catalog_;
  bool firstFile_ {true};
  RootInputFileSharedPtr rootFile_ {nullptr};
  std::vector<std::shared_ptr<FileIndex>> fileIndexes_;
  int eventsRemainingInFile_ {};
  EventID origEventID_ {};
  EventNumber_t eventsToSkip_;
  bool const noEventSort_;
  bool const skipBadFiles_;
  unsigned int const treeCacheSize_;
  int64_t const treeMaxVirtualSize_;
  int64_t const saveMemoryObjectThreshold_;
  bool const delayedReadEventProducts_;
  bool const delayedReadSubRunProducts_;
  bool const delayedReadRunProducts_;
  int forcedRunOffset_ {};
  RunNumber_t setRun_ {};
  GroupSelectorRules groupSelectorRules_;
  std::shared_ptr<DuplicateChecker> duplicateChecker_ {nullptr};
  bool const dropDescendants_;
  bool const readParameterSets_;
  RootInputFileSharedPtr rootFileForLastReadEvent_;
  FastCloningInfoProvider fastCloningInfo_;
  InputSource::ProcessingMode processingMode_;
  ProcessConfiguration const& processConfiguration_;
  std::vector<std::vector<std::string>> secondaryFileNames_ {};
  MasterProductRegistry& mpr_;
  bool pendingClose_ {false};
};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootInputFileSequence_h */
