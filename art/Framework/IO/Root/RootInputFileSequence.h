#ifndef art_Framework_IO_Root_RootInputFileSequence_h
#define art_Framework_IO_Root_RootInputFileSequence_h
// vim: set sw=2:

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/GroupSelectorRules.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/IO/Root/DuplicateChecker.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/OptionalSequence.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"

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

  virtual
  ~RootInputFileSequence();

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
    Atom<std::string> fileMatchMode { Name("fileMatchMode"), "permissive" };
    OptionalAtom<RunNumber_t> setRunNumber { Name("setRunNumber") };

  };

  RootInputFileSequence(fhicl::TableFragment<Config> const&, InputFileCatalog&,
                        FastCloningInfoProvider const&,
                        InputSource::ProcessingMode,
                        MasterProductRegistry&, ProcessConfiguration const&);
  void
  endJob();

  std::shared_ptr<FileBlock>
  readFile_();

  std::unique_ptr<RootInputFile>
  openSecondaryFile(int idx, std::string const& name,
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

  std::shared_ptr<RunPrincipal>
  readIt(RunID const&);

  std::shared_ptr<RunPrincipal>
  readRun_();

  std::vector<std::shared_ptr<RunPrincipal>>
  readRunFromSecondaryFiles_();

  std::shared_ptr<SubRunPrincipal>
  readIt(SubRunID const&, std::shared_ptr<RunPrincipal>);

  std::shared_ptr<SubRunPrincipal>
  readSubRun_(std::shared_ptr<RunPrincipal>);

  std::vector<std::shared_ptr<SubRunPrincipal>>
  readSubRunFromSecondaryFiles_(std::shared_ptr<RunPrincipal>);

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

  std::vector<SubRunID> const&
  whichSubRunsToSkip()
  {
    return whichSubRunsToSkip_;
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

private: // MEMBER FUNCTIONS

  void
  initFile(bool skipBadFiles, bool initMPR = false);

  bool
  nextFile();

  bool
  previousFile();

  void
  rewindFile();

  std::unique_ptr<EventPrincipal>
  readCurrentEvent();

  std::vector<FileCatalogItem> const&
  fileCatalogItems() const;

  ProcessConfiguration const&
  processConfiguration() const;

  bool
  primary() const;

  void
  logFileAction(const char* msg, std::string const& file);

private: // MEMBER DATA

  InputFileCatalog& catalog_;
  bool firstFile_;
  RootInputFileSharedPtr rootFile_;
  BranchDescription::MatchMode matchMode_;
  std::vector<std::shared_ptr<FileIndex>> fileIndexes_;
  int eventsRemainingInFile_;
  EventID origEventID_;
  EventNumber_t eventsToSkip_;
  std::vector<SubRunID> whichSubRunsToSkip_; // FIXME: unused
  bool const noEventSort_;
  bool const skipBadFiles_;
  unsigned int const treeCacheSize_;
  int64_t const treeMaxVirtualSize_;
  int64_t const saveMemoryObjectThreshold_;
  bool const delayedReadSubRunProducts_;
  bool const delayedReadRunProducts_;
  int forcedRunOffset_;
  RunNumber_t setRun_;
  GroupSelectorRules groupSelectorRules_;
  std::shared_ptr<DuplicateChecker> duplicateChecker_;
  bool const dropDescendants_;
  bool const readParameterSets_;
  RootInputFileSharedPtr rootFileForLastReadEvent_;
  FastCloningInfoProvider fastCloningInfo_;
  InputSource::ProcessingMode processingMode_;
  ProcessConfiguration const& processConfiguration_;
  std::vector<std::vector<std::string>> secondaryFileNames_;
  MasterProductRegistry& mpr_;

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootInputFileSequence_h */
