#ifndef IOPool_Input_RootFile_h
#define IOPool_Input_RootFile_h

/*----------------------------------------------------------------------

RootFile.h // used by ROOT input sources

----------------------------------------------------------------------*/

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/IO/Input/RootTree.h"
#include "art/Persistency/Provenance/BranchChildren.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/EventProcessHistoryID.h" // backward compatibility
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/FileID.h"
#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunID.h"

#include "boost/array.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/utility.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

class TFile;


namespace edm {

  //------------------------------------------------------------
  // Class RootFile: supports file reading.

  class DuplicateChecker;
  class ProvenanceAdaptor;
  class GroupSelectorRules;

  class RootFile : private boost::noncopyable {
  public:
    typedef boost::array<RootTree *, NumBranchTypes> RootTreePtrArray;
    RootFile(std::string const& fileName,
	     std::string const& catalogName,
	     ProcessConfiguration const& processConfiguration,
	     std::string const& logicalFileName,
	     boost::shared_ptr<TFile> filePtr,
	     RunNumber_t const& startAtRun,
	     SubRunNumber_t const& startAtSubRun,
	     EventNumber_t const& startAtEvent,
	     unsigned int eventsToSkip,
	     std::vector<SubRunID> const& whichSubRunsToSkip,
	     int remainingEvents,
	     int remainingSubRuns,
	     unsigned int treeCacheSize,
             int treeMaxVirtualSize,
	     InputSource::ProcessingMode processingMode,
	     int forcedRunOffset,
	     std::vector<EventID> const& whichEventsToProcess,
             bool noEventSort,
	     GroupSelectorRules const& groupSelectorRules,
             bool dropMergeable,
             boost::shared_ptr<DuplicateChecker> duplicateChecker,
             bool dropDescendantsOfDroppedProducts);
    void reportOpened();
    void close(bool reallyClose);
    std::auto_ptr<EventPrincipal> readCurrentEvent(
	boost::shared_ptr<ProductRegistry const> pReg);
    std::auto_ptr<EventPrincipal> readEvent(
	boost::shared_ptr<ProductRegistry const> pReg);
    boost::shared_ptr<SubRunPrincipal> readSubRun(
	boost::shared_ptr<ProductRegistry const> pReg,
	boost::shared_ptr<RunPrincipal> rp);
    std::string const& file() const {return file_;}
    boost::shared_ptr<RunPrincipal> readRun(boost::shared_ptr<ProductRegistry const> pReg);
    boost::shared_ptr<ProductRegistry const> productRegistry() const {return productRegistry_;}
    BranchIDListRegistry::collection_type const& branchIDLists() {return *branchIDLists_;}
    EventAuxiliary const& eventAux() const {return eventAux_;}
    SubRunAuxiliary const& subRunAux() {return subRunAux_;}
    RunAuxiliary const& runAux() const {return runAux_;}
    EventID const& eventID() const {return eventAux().id();}
    RootTreePtrArray & treePointers() {return treePointers_;}
    RootTree const& eventTree() const {return eventTree_;}
    RootTree const& subRunTree() const {return subRunTree_;}
    RootTree const & runTree() const {return runTree_;}
    FileFormatVersion fileFormatVersion() const {return fileFormatVersion_;}
    bool fastClonable() const {return fastClonable_;}
    boost::shared_ptr<FileBlock> createFileBlock() const;
    bool setEntryAtEvent(RunNumber_t run, SubRunNumber_t subRun, EventNumber_t event, bool exact);
    bool setEntryAtSubRun(SubRunID const& subRun);
    bool setEntryAtRun(RunID const& run);
    void setAtEventEntry(FileIndex::EntryNumber_t entry);
    void rewind() {
      fileIndexIter_ = fileIndexBegin_;
      eventTree_.rewind();
      subRunTree_.rewind();
      runTree_.rewind();
    }
    void setToLastEntry() {
      fileIndexIter_ = fileIndexEnd_;
    }

    unsigned int eventsToSkip() const {return eventsToSkip_;}
    int skipEvents(int offset);
    int setForcedRunOffset(RunNumber_t const& forcedRunNumber);
    bool nextEventEntry() {return eventTree_.next();}
    FileIndex::EntryType getEntryType() const;
    FileIndex::EntryType getEntryTypeSkippingDups();
    FileIndex::EntryType getNextEntryTypeWanted();
    boost::shared_ptr<FileIndex> fileIndexSharedPtr() const {
      return fileIndexSharedPtr_;
    }

  private:
    bool setIfFastClonable(int remainingEvents, int remainingSubRuns) const;
    void validateFile();
    void fillFileIndex();
    void fillEventAuxiliary();
    void fillHistory();
    void fillSubRunAuxiliary();
    void fillRunAuxiliary();
    void overrideRunNumber(RunID & id);
    void overrideRunNumber(SubRunID & id);
    void overrideRunNumber(EventID & id, bool isRealData);
    std::string const& newBranchToOldBranch(std::string const& newBranch) const;
    void dropOnInput(GroupSelectorRules const& rules, bool dropDescendants, bool dropMergeable);
    void readParentageTree();
    void readEntryDescriptionTree();
    void readEventHistoryTree();

    void initializeDuplicateChecker();

    template <typename T>
    boost::shared_ptr<BranchMapper> makeBranchMapper(RootTree & rootTree, BranchType const& type) const;

    boost::shared_ptr<BranchMapper> makeBranchMapperInOldRelease(RootTree & rootTree, BranchType const& type) const;

    std::string const file_;
    std::string const logicalFile_;
    std::string const catalog_;
    ProcessConfiguration const& processConfiguration_;
    boost::shared_ptr<TFile> filePtr_;
    FileFormatVersion fileFormatVersion_;
    FileID fid_;
    boost::shared_ptr<FileIndex> fileIndexSharedPtr_;
    FileIndex & fileIndex_;
    FileIndex::const_iterator fileIndexBegin_;
    FileIndex::const_iterator fileIndexEnd_;
    FileIndex::const_iterator fileIndexIter_;
    std::vector<EventProcessHistoryID> eventProcessHistoryIDs_;  // backward compatibility
    std::vector<EventProcessHistoryID>::const_iterator eventProcessHistoryIter_; // backward compatibility
    RunNumber_t startAtRun_;
    SubRunNumber_t startAtSubRun_;
    EventNumber_t startAtEvent_;
    unsigned int eventsToSkip_;
    std::vector<SubRunID> whichSubRunsToSkip_;
    std::vector<EventID> whichEventsToProcess_;
    std::vector<EventID>::const_iterator eventListIter_;
    bool noEventSort_;
    bool fastClonable_;
    EventAuxiliary eventAux_;
    SubRunAuxiliary subRunAux_;
    RunAuxiliary runAux_;
    RootTree eventTree_;
    RootTree subRunTree_;
    RootTree runTree_;
    RootTreePtrArray treePointers_;
    boost::shared_ptr<ProductRegistry const> productRegistry_;
    boost::shared_ptr<BranchIDListRegistry::collection_type const> branchIDLists_;
    InputSource::ProcessingMode processingMode_;
    int forcedRunOffset_;
    std::map<std::string, std::string> newBranchToOldBranch_;
    TTree * eventHistoryTree_;
    boost::shared_ptr<History> history_;
    boost::shared_ptr<BranchChildren> branchChildren_;
    boost::shared_ptr<DuplicateChecker> duplicateChecker_;
    boost::shared_ptr<ProvenanceAdaptor> provenanceAdaptor_;
  }; // class RootFile

  template <typename T>
  inline
  boost::shared_ptr<BranchMapper>
  RootFile::makeBranchMapper(RootTree & rootTree, BranchType const& type) const {
    if (fileFormatVersion_.value_ < 8) {
      return makeBranchMapperInOldRelease(rootTree, type);
    }
    boost::shared_ptr<BranchMapper> bm = rootTree.makeBranchMapper<T>();
    return bm;
  }

}  // namespace edm

#endif  // IOPool_Input_RootFile_h
