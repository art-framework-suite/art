#ifndef art_Framework_IO_Root_RootInputFileSequence_h
#define art_Framework_IO_Root_RootInputFileSequence_h

// ======================================================================
//
// RootInputFileSequence - This is an InputSource
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

  class RootInputFile;
  class FileCatalogItem;
  class InputFileCatalog;
  class FileIndex;
  class DuplicateChecker;

  class RootInputFileSequence : private boost::noncopyable {
  public:
    explicit RootInputFileSequence(fhicl::ParameterSet const& pset,
                                   InputFileCatalog const& catalog,
                                   bool primarySequence,
                                   FastCloningInfoProvider const &fcip,
                                   InputSource::ProcessingMode pMode,
                                   ProductRegistry &pReg,
                                   ProcessConfiguration const &processConfig);
    virtual ~RootInputFileSequence();

    typedef std::shared_ptr<RootInputFile> RootInputFileSharedPtr;
    typedef input::EntryNumber EntryNumber;
    std::auto_ptr<EventPrincipal> readEvent_();
    std::shared_ptr<SubRunPrincipal> readSubRun_(std::shared_ptr<RunPrincipal> rp);
    std::shared_ptr<RunPrincipal> readRun_();
    std::shared_ptr<FileBlock> readFile_();
    void closeFile_();
    void endJob();
    input::ItemType getNextItemType();
    std::auto_ptr<EventPrincipal> readIt(EventID const& id, bool exact = false);
    std::shared_ptr<SubRunPrincipal> readIt(SubRunID const& id, std::shared_ptr<RunPrincipal> rp);
    std::shared_ptr<RunPrincipal> readIt(RunID const& run);
    void skip(int offset);
    void rewind_();
    ProductRegistry const& fileProductRegistry() const;
    EventID seekToEvent(EventID const &eID, bool exact = false);
    EventID seekToEvent(off_t offset, bool exact = false);
    RootInputFileSharedPtr rootFileForLastReadEvent() const {
      return rootFileForLastReadEvent_;
    }
    RootInputFileSharedPtr rootFile() const {
      return rootFile_;
    }

  private:
    void initFile(bool skipBadFiles);
    bool nextFile();
    bool previousFile();
    void rewindFile();
    std::auto_ptr<EventPrincipal> readCurrentEvent();
    std::vector<FileCatalogItem> const& fileCatalogItems() const;

    cet::exempt_ptr<ProductRegistry const> productRegistry() const;
    ProcessConfiguration const& processConfiguration() const;
    ProductRegistry & productRegistryUpdate() const;
    bool primary() const;
    void logFileAction(const char* msg, std::string const& file);

    InputFileCatalog const& catalog_;
    bool firstFile_;
    std::vector<FileCatalogItem>::const_iterator fileIterBegin_;
    std::vector<FileCatalogItem>::const_iterator fileIterEnd_;
    std::vector<FileCatalogItem>::const_iterator fileIter_;
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
    ProductRegistry &productRegistry_;
    ProcessConfiguration const &processConfiguration_;
  };  // RootInputFileSequence

}  // art

// ======================================================================

#endif /* art_Framework_IO_Root_RootInputFileSequence_h */

// Local Variables:
// mode: c++
// End:
