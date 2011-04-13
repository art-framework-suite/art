#ifndef art_Framework_IO_Root_RootInputFile_h
#define art_Framework_IO_Root_RootInputFile_h

// ======================================================================
//
// RootInputFile - used by ROOT input sources
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/IO/Root/RootTree.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Persistency/Provenance/BranchChildren.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "boost/array.hpp"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "cetlib/exempt_ptr.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

// ----------------------------------------------------------------------

class TFile;


namespace art {

  //------------------------------------------------------------
  // Class RootInputFile: supports file reading.

  class DuplicateChecker;
  class GroupSelectorRules;

  class RootInputFile : private boost::noncopyable {
  public:
    typedef boost::array<RootTree *, NumBranchTypes> RootTreePtrArray;
    RootInputFile(std::string const& fileName,
             std::string const& catalogName,
             ProcessConfiguration const& processConfiguration,
             std::string const& logicalFileName,
             boost::shared_ptr<TFile> filePtr,
             EventID const &origEventID,
             unsigned int eventsToSkip,
             std::vector<SubRunID> const& whichSubRunsToSkip,
             FastCloningInfoProvider const &fcip,
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
        cet::exempt_ptr<ProductRegistry const> pReg);
    std::auto_ptr<EventPrincipal> readEvent(
        cet::exempt_ptr<ProductRegistry const> pReg);
    boost::shared_ptr<SubRunPrincipal> readSubRun(
        cet::exempt_ptr<ProductRegistry const> pReg,
        boost::shared_ptr<RunPrincipal> rp);
    std::string const& file() const {return file_;}
    boost::shared_ptr<RunPrincipal> readRun(cet::exempt_ptr<ProductRegistry const> pReg);
    cet::exempt_ptr<ProductRegistry const> productRegistry() const {return productRegistry_;}
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
    bool setEntryAtEvent(EventID const &eID, bool exact);
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
    EventID eventIDForFileIndexPosition() const;

  private:
    bool setIfFastClonable(FastCloningInfoProvider const &fcip) const;
    void validateFile();
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
    void readEventHistoryTree();

    void initializeDuplicateChecker();

    template <typename T>
    boost::shared_ptr<BranchMapper> makeBranchMapper(RootTree & rootTree, BranchType const& type) const;

    std::string const file_;
    std::string const logicalFile_;
    std::string const catalog_;
    ProcessConfiguration const& processConfiguration_;
    boost::shared_ptr<TFile> filePtr_;
    FileFormatVersion fileFormatVersion_;
    boost::shared_ptr<FileIndex> fileIndexSharedPtr_;
    FileIndex & fileIndex_;
    FileIndex::const_iterator fileIndexBegin_;
    FileIndex::const_iterator fileIndexEnd_;
    FileIndex::const_iterator fileIndexIter_;
    EventID origEventID_;
    EventNumber_t eventsToSkip_;
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
    cet::exempt_ptr<ProductRegistry const> productRegistry_;
    boost::shared_ptr<BranchIDListRegistry::collection_type const> branchIDLists_;
    InputSource::ProcessingMode processingMode_;
    int forcedRunOffset_;
    std::map<std::string, std::string> newBranchToOldBranch_;
    TTree * eventHistoryTree_;
    boost::shared_ptr<History> history_;
    boost::shared_ptr<BranchChildren> branchChildren_;
    boost::shared_ptr<DuplicateChecker> duplicateChecker_;
  }; // RootInputFile

  template <typename T>
  inline
  boost::shared_ptr<BranchMapper>
  RootInputFile::makeBranchMapper(RootTree & rootTree, BranchType const& type) const {
    boost::shared_ptr<BranchMapper> bm = rootTree.makeBranchMapper<T>();
    return bm;
  }

}  // art

// ======================================================================

#endif /* art_Framework_IO_Root_RootInputFile_h */

// Local Variables:
// mode: c++
// End:
