#ifndef art_Framework_IO_Root_RootInputFileSequence_h
#define art_Framework_IO_Root_RootInputFileSequence_h

// ======================================================================
//
// RootInputFileSequence: This encapsulates a sequence of RootInputFiles,
// and keeps track of the current location within the seqeuence.
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/GroupSelectorRules.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "boost/noncopyable.hpp"
#include "boost/noncopyable.hpp"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"
#include <string>
#include <vector>

// ----------------------------------------------------------------------

namespace art {

  class DuplicateChecker;
  class FileCatalogItem;
  class FileIndex;
  class InputFileCatalog;
  class MasterProductRegistry;
  class RootInputFile;

  class RootInputFileSequence : private boost::noncopyable {
  public:
    explicit RootInputFileSequence(fhicl::ParameterSet const& pset,
                                   InputFileCatalog & catalog,
                                   bool primarySequence,
                                   FastCloningInfoProvider const &fcip,
                                   InputSource::ProcessingMode pMode,
                                   MasterProductRegistry &pReg,
                                   ProcessConfiguration const &processConfig);
    virtual ~RootInputFileSequence();

    typedef std::shared_ptr<RootInputFile> RootInputFileSharedPtr;
    typedef input::EntryNumber EntryNumber;
    std::unique_ptr<EventPrincipal> readEvent_();
    std::shared_ptr<SubRunPrincipal> readSubRun_(std::shared_ptr<RunPrincipal> rp);
    std::shared_ptr<RunPrincipal> readRun_();
    std::shared_ptr<FileBlock> readFile_(MasterProductRegistry&);
    void closeFile_();
    void endJob();
    input::ItemType getNextItemType();
    std::unique_ptr<EventPrincipal> readIt(EventID const& id, MasterProductRegistry& mpr, bool exact = false);
    std::shared_ptr<SubRunPrincipal> readIt(SubRunID const& id, std::shared_ptr<RunPrincipal> rp);
    std::shared_ptr<RunPrincipal> readIt(RunID const& run);
    void skip(int offset, MasterProductRegistry&);
    void rewind_();
    EventID seekToEvent(EventID const &eID, MasterProductRegistry& mpr, bool exact = false);
    EventID seekToEvent(off_t offset, MasterProductRegistry& mpr, bool exact = false);
    RootInputFileSharedPtr rootFileForLastReadEvent() const {
      return rootFileForLastReadEvent_;
    }
    RootInputFileSharedPtr rootFile() const {
      return rootFile_;
    }

  private:
    void initFile(bool skipBadFiles);
    bool nextFile(MasterProductRegistry&);
    bool previousFile(MasterProductRegistry&);
    void rewindFile();
    std::unique_ptr<EventPrincipal> readCurrentEvent();
    std::vector<FileCatalogItem> const& fileCatalogItems() const;

    ProcessConfiguration const& processConfiguration() const;
    bool primary() const;
    void logFileAction(const char* msg, std::string const& file);
    void mergeMPR(MasterProductRegistry & mpr);

    InputFileCatalog & catalog_;
    bool firstFile_;
    RootInputFileSharedPtr rootFile_;
    BranchDescription::MatchMode matchMode_;
    std::vector<std::shared_ptr<FileIndex> > fileIndexes_;

    int eventsRemainingInFile_;
    EventID origEventID_;
    EventNumber_t eventsToSkip_;
    std::vector<SubRunID> whichSubRunsToSkip_;
    std::vector<EventID> eventsToProcess_;
    bool noEventSort_;
    bool skipBadFiles_;
    unsigned int treeCacheSize_;
    int const treeMaxVirtualSize_;
    int forcedRunOffset_;
    RunNumber_t setRun_;
    GroupSelectorRules groupSelectorRules_;
    bool primarySequence_;
    std::shared_ptr<DuplicateChecker> duplicateChecker_;
    bool dropDescendants_;
    RootInputFileSharedPtr rootFileForLastReadEvent_;
    FastCloningInfoProvider fastCloningInfo_;
    InputSource::ProcessingMode processingMode_;
    ProcessConfiguration const &processConfiguration_;
  };  // RootInputFileSequence

}  // art

// ======================================================================

#endif /* art_Framework_IO_Root_RootInputFileSequence_h */

// Local Variables:
// mode: c++
// End:
