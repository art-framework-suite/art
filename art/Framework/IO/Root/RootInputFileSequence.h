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
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "boost/noncopyable.hpp"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "cetlib/exempt_ptr.h"
#include <memory>
#include <string>
#include <vector>

// ----------------------------------------------------------------------

namespace CLHEP {
  class RandFlat;
}

namespace art {

  class RootInput;
  class RootInputFile;
  class FileCatalogItem;
  class InputFileCatalog;
  class FileIndex;
  class DuplicateChecker;

  class RootInputFileSequence : private boost::noncopyable {
  public:
    explicit RootInputFileSequence(fhicl::ParameterSet const& pset,
                                   RootInput const& input,
                                   InputFileCatalog const& catalog,
                                   bool primarySequence);
    virtual ~RootInputFileSequence();

    typedef boost::shared_ptr<RootInputFile> RootInputFileSharedPtr;
    typedef input::EntryNumber EntryNumber;
    std::auto_ptr<EventPrincipal> readEvent_();
    boost::shared_ptr<SubRunPrincipal> readSubRun_();
    boost::shared_ptr<RunPrincipal> readRun_();
    boost::shared_ptr<FileBlock> readFile_();
    void closeFile_();
    void endJob();
    input::ItemType getNextItemType();
    std::auto_ptr<EventPrincipal> readIt(EventID const& id, bool exact = false);
    boost::shared_ptr<SubRunPrincipal> readIt(SubRunID const& id);
    boost::shared_ptr<RunPrincipal> readIt(RunID const& run);
    void skip(int offset);
    void rewind_();
    ProductRegistry const& fileProductRegistry() const;
  private:
    void initFile(bool skipBadFiles);
    bool nextFile();
    bool previousFile();
    void rewindFile();
    std::auto_ptr<EventPrincipal> readCurrentEvent();
    std::vector<FileCatalogItem> const& fileCatalogItems() const;

    cet::exempt_ptr<ProductRegistry const> productRegistry() const;
    boost::shared_ptr<RunPrincipal> runPrincipal() const;
    ProcessConfiguration const& processConfiguration() const;
    ProductRegistry & productRegistryUpdate() const;
    int remainingEvents() const;
    int remainingSubRuns() const;
    bool primary() const;
    void logFileAction(const char* msg, std::string const& file);

    RootInput const& input_;
    InputFileCatalog const& catalog_;
    bool firstFile_;
    std::vector<FileCatalogItem>::const_iterator fileIterBegin_;
    std::vector<FileCatalogItem>::const_iterator fileIterEnd_;
    std::vector<FileCatalogItem>::const_iterator fileIter_;
    RootInputFileSharedPtr rootFile_;
    BranchDescription::MatchMode matchMode_;
    CLHEP::RandFlat * flatDistribution_;
    std::vector<boost::shared_ptr<FileIndex> > fileIndexes_;

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
    bool randomAccess_;
    boost::shared_ptr<DuplicateChecker> duplicateChecker_;
    bool dropDescendants_;
  };  // RootInputFileSequence

}  // art

// ======================================================================

#endif /* art_Framework_IO_Root_RootInputFileSequence_h */

// Local Variables:
// mode: c++
// End:
