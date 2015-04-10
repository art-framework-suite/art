// ======================================================================
//
// Class RootOutputFile
//
// ======================================================================

#include "art/Framework/IO/Root/RootOutputFile.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/GetFileFormatVersion.h"
#include "art/Framework/IO/Root/rootNames.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Provenance/BranchChildren.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/ParameterSetBlob.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ParentageRegistry.h"
#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/RootDB/SQLErrMsg.h"
#include "art/Persistency/RootDB/SQLite3Wrapper.h"
#include "art/Utilities/Exception.h"
#include "art/Version/GetReleaseVersion.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "cetlib/canonical_string.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/algorithm"
#include "cpp0x/utility"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include "Rtypes.h"
#include "TClass.h"
#include "TFile.h"
#include "TTree.h"

#include <iomanip>
#include <sstream>
#include <vector>
#include <utility>

using namespace cet;
using namespace std;
using art::rootNames::metaBranchRootName;

namespace {
  void insert_md_row(sqlite3_stmt * stmt,
                     std::pair<std::string, std::string> const & kv)
  {
    std::string const & theName  (kv.first);
    std::string const & theValue (kv.second);
    sqlite3_bind_text(stmt, 1, theName.c_str(),
                      theName.size() + 1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, theValue.c_str(),
                      theValue.size() + 1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_reset(stmt);
  }
}

namespace art {

  RootOutputFile::RootOutputFile(RootOutput *om, string const& fileName)
    :
    file_(fileName),
    om_(om),
    currentlyFastCloning_(true),
    filePtr_(TFile::Open(file_.c_str(), "recreate", "", om_->compressionLevel())),
    fileIndex_(),
    eventEntryNumber_(0LL),
    subRunEntryNumber_(0LL),
    runEntryNumber_(0LL),
    metaDataTree_(0),
    parentageTree_(0),
    eventHistoryTree_(0),
    pEventAux_(0),
    pSubRunAux_(0),
    pRunAux_(0),
    eventProductProvenanceVector_(),
    subRunProductProvenanceVector_(),
    runProductProvenanceVector_(),
    pEventProductProvenanceVector_(&eventProductProvenanceVector_),
    pSubRunProductProvenanceVector_(&subRunProductProvenanceVector_),
    pRunProductProvenanceVector_(&runProductProvenanceVector_),
    pHistory_(0),
    eventTree_(static_cast<EventPrincipal *>(0),
               filePtr_, InEvent, pEventAux_, pEventProductProvenanceVector_,
               om_->basketSize(), om_->splitLevel(), om_->treeMaxVirtualSize(),
               om_->saveMemoryObjectThreshold()),
    subRunTree_(static_cast<SubRunPrincipal *>(0),
                filePtr_, InSubRun, pSubRunAux_, pSubRunProductProvenanceVector_,
                om_->basketSize(), om_->splitLevel(), om_->treeMaxVirtualSize(),
                om_->saveMemoryObjectThreshold()),
    runTree_(static_cast<RunPrincipal *>(0),
             filePtr_, InRun, pRunAux_, pRunProductProvenanceVector_,
             om_->basketSize(), om_->splitLevel(), om_->treeMaxVirtualSize(),
             om_->saveMemoryObjectThreshold()),
    treePointers_(),
    dataTypeReported_(false),
    metaDataHandle_(filePtr_.get(), "RootFileDB", SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE)
  {
    treePointers_[InEvent] = &eventTree_;
    treePointers_[InSubRun]  = &subRunTree_;
    treePointers_[InRun]   = &runTree_;

    for (int i = InEvent; i < NumBranchTypes; ++i) {
      BranchType branchType = static_cast<BranchType>(i);
      for (OutputItemList::const_iterator it = om_->selectedOutputItemList()[branchType].begin(),
                                       itEnd = om_->selectedOutputItemList()[branchType].end();
           it != itEnd; ++it) {
        treePointers_[branchType]->addBranch(*it->branchDescription_,
                                             it->product_);
      }
    }
    // Don't split metadata tree or event description tree
    metaDataTree_         = RootOutputTree::makeTTree(filePtr_.get(), rootNames::metaDataTreeName(), 0);
    parentageTree_ = RootOutputTree::makeTTree(filePtr_.get(), rootNames::parentageTreeName(), 0);

    // Create the tree that will carry (event) History objects.
    eventHistoryTree_     = RootOutputTree::makeTTree(filePtr_.get(), rootNames::eventHistoryTreeName(), om_->splitLevel());
    if (!eventHistoryTree_)
      throw art::Exception(art::errors::FatalRootError)
        << "Failed to create the tree for History objects\n";

    if (! eventHistoryTree_->Branch(rootNames::eventHistoryBranchName().c_str(), &pHistory_, om_->basketSize(), 0))
      throw art::Exception(art::errors::FatalRootError)
        << "Failed to create a branch for History in the output file\n";
  }

  void RootOutputFile::beginInputFile(FileBlock const& fb, bool fastClone) {
    auto const origCurrentlyFastCloning = currentlyFastCloning_;
    currentlyFastCloning_ = om_->fastCloning() && fastClone;
    if (currentlyFastCloning_ && ! eventTree_.checkSplitLevelAndBasketSize(fb.tree())) {
      mf::LogWarning("FastCloning")
        << "Fast cloning deactivated for this input file due to "
        << "splitting level and/or basket size.";
      currentlyFastCloning_ = false;
    }
    if (currentlyFastCloning_ and ! origCurrentlyFastCloning) {
      mf::LogWarning("FastCloning")
        << "Fast cloning reactivated for this input file.";
    }
    eventTree_.beginInputFile(currentlyFastCloning_);
    eventTree_.fastCloneTree(fb.tree());
  }

  void RootOutputFile::respondToCloseInputFile(FileBlock const&) {
    eventTree_.setEntries();
    subRunTree_.setEntries();
    runTree_.setEntries();
  }

  bool RootOutputFile::shouldWeCloseFile() const {
    unsigned int const oneK = 1024;
    Long64_t size = filePtr_->GetSize()/oneK;
    return(size >= om_->maxFileSize_);
  }

  void RootOutputFile::writeOne(EventPrincipal const& e) {
    // Auxiliary branch
    pEventAux_ = &e.aux();

    // Because getting the data may cause an exception to be thrown we want to do that
    // first before writing anything to the file about this event
    // NOTE: pEventAux_ must be set before calling fillBranches since it gets written out
    // in that routine.
    fillBranches(InEvent, e, pEventProductProvenanceVector_);

    // History branch
    History historyForOutput(e.history());
    historyForOutput.addEventSelectionEntry(om_->selectorConfig());
    pHistory_ = &historyForOutput;
    int sz = eventHistoryTree_->Fill();
    if ( sz <= 0)
      throw art::Exception(art::errors::FatalRootError)
        << "Failed to fill the History tree for event: " << e.id()
        << "\nTTree::Fill() returned " << sz << " bytes written." << endl;

    // Add the dataType to the job report if it hasn't already been done
    if(!dataTypeReported_) {
      string dataType("MC");
      if(pEventAux_->isRealData())  dataType = "Data";
      dataTypeReported_ = true;
    }

    pHistory_ = & e.history();

    // Add event to index
    fileIndex_.addEntry(pEventAux_->id(), eventEntryNumber_);
    ++eventEntryNumber_;

    // Report event written
  }

  void RootOutputFile::writeSubRun(SubRunPrincipal const& sr) {
    // Auxiliary branch
    pSubRunAux_ = &sr.aux();
    // Add subRun to index.
    fileIndex_.addEntry(EventID::invalidEvent(pSubRunAux_->id()), subRunEntryNumber_);
    ++subRunEntryNumber_;
    fillBranches(InSubRun, sr, pSubRunProductProvenanceVector_);
  }

  void RootOutputFile::writeRun(RunPrincipal const& r) {
    // Auxiliary branch
    pRunAux_ = &r.aux();
    // Add run to index.
    fileIndex_.addEntry(EventID::invalidEvent(pRunAux_->id()),runEntryNumber_);
    ++runEntryNumber_;
    fillBranches(InRun, r, pRunProductProvenanceVector_);
  }

  void RootOutputFile::writeParentageRegistry() {
    ParentageID const* hash(0);
    Parentage const*   desc(0);

    if (!parentageTree_->Branch(rootNames::parentageIDBranchName().c_str(),
                                        &hash, om_->basketSize(), 0))
      throw art::Exception(art::errors::FatalRootError)
        << "Failed to create a branch for ParentageIDs in the output file";

    if (!parentageTree_->Branch(rootNames::parentageBranchName().c_str(),
                                        &desc, om_->basketSize(), 0))
      throw art::Exception(art::errors::FatalRootError)
        << "Failed to create a branch for Parentages in the output file";

    for (ParentageRegistry::const_iterator
           i = ParentageRegistry::begin(),
           e = ParentageRegistry::end();
         i != e;
         ++i) {
        hash = const_cast<ParentageID*>(&(i->first)); // cast needed because keys are const
        desc = &(i->second);
        parentageTree_->Fill();
      }
  }

  void RootOutputFile::writeFileFormatVersion() {
    FileFormatVersion fileFormatVersion(getFileFormatVersion(), getFileFormatEra());
    FileFormatVersion * pFileFmtVsn = &fileFormatVersion;
    TBranch* b = metaDataTree_->Branch(metaBranchRootName<FileFormatVersion>(), &pFileFmtVsn, om_->basketSize(), 0);
    assert(b);
    b->Fill();
  }

  void RootOutputFile::writeFileIndex() {
    fileIndex_.sortBy_Run_SubRun_Event();
    FileIndex *findexPtr = &fileIndex_;
    TBranch* b = metaDataTree_->Branch(metaBranchRootName<FileIndex>(), &findexPtr, om_->basketSize(), 0);
    assert(b);
    b->Fill();
  }

  void RootOutputFile::writeEventHistory() {
    RootOutputTree::writeTTree(eventHistoryTree_);
  }

  void RootOutputFile::writeProcessConfigurationRegistry() {
    // We don't do this yet; currently we're storing a slightly bloated ProcessHistoryRegistry.
  }

  void RootOutputFile::writeProcessHistoryRegistry() {
    ProcessHistoryMap const & r = ProcessHistoryRegistry::get();
    ProcessHistoryMap * p = & const_cast<ProcessHistoryMap &>(r);
    TBranch* b = metaDataTree_->Branch(metaBranchRootName<ProcessHistoryMap>(), &p, om_->basketSize(), 0);
    assert(b);
    b->Fill();
  }

  void RootOutputFile::writeBranchIDListRegistry() {
    BranchIDLists *p = &BranchIDListRegistry::instance()->data();
    TBranch* b = metaDataTree_->Branch(metaBranchRootName<BranchIDLists>(), &p, om_->basketSize(), 0);
    assert(b);
    b->Fill();
  }

  void
  RootOutputFile::
  writeFileCatalogMetadata(FileStatsCollector const & stats,
                           FileCatalogMetadata::collection_type const & md,
                           FileCatalogMetadata::collection_type const & ssmd)
  {
    SQLErrMsg errMsg;
    // ID is declared auto-increment, so don't specify it when filling a
    // row.
    sqlite3_exec(metaDataHandle_,
                 "BEGIN TRANSACTION; " // +
                 "DROP TABLE IF EXISTS FileCatalog_metadata; " // +
                 "CREATE TABLE FileCatalog_metadata(ID INTEGER PRIMARY KEY," //+
                 "                                  Name, Value); " // +
                 "COMMIT;",
                 0,
                 0,
                 errMsg);
    errMsg.throwIfError();

    sqlite3_exec(metaDataHandle_, "BEGIN TRANSACTION;", 0, 0, errMsg);
    sqlite3_stmt *stmt = 0;
    sqlite3_prepare_v2(metaDataHandle_,
                       "INSERT INTO FileCatalog_metadata(Name, Value) VALUES(?, ?);",
                       -1, &stmt, NULL);
    for ( auto const & kv : md ) {
      insert_md_row(stmt, kv);
    }
    ////////////////////////////////////
    // Add our own specific information:

    // File format and friends.
    insert_md_row(stmt, { "file_format", "\"artroot\"" });
    insert_md_row(stmt, { "file_format_era",
          cet::canonical_string(getFileFormatEra()) });
    insert_md_row(stmt, { "file_format_version",
          std::to_string(getFileFormatVersion()) });

    namespace bpt = boost::posix_time;
    // File start time.
    insert_md_row(stmt,
                  { "start_time",
                      cet::canonical_string(bpt::to_iso_extended_string(stats.outputFileOpenTime())) });

    // File "end" time: now, since file is (of course) not actually
    // closed yet.
    insert_md_row(stmt,
                  { "end_time",
                      cet::canonical_string(bpt::to_iso_extended_string(boost::posix_time::second_clock::universal_time()))
                  });

    // Run / subRun information.
    if (!stats.seenSubRuns().empty()) {
      decltype(md.crbegin()) it;
      if ((it =
           std::find_if(md.crbegin(),
                        md.crend(),
                        [](std::pair<std::string, std::string> const & p)
                        { return (p.first == "run_type"); })) != md.crend()) {
        std::ostringstream srstring;
        srstring << "[ ";
        bool first = true;
        for (auto const & srid : stats.seenSubRuns()) {
          if (first) {
            first = false;
          } else {
            srstring << ", ";
          }
          srstring << "[ "
                   << srid.run()
                   << ", "
                   << srid.subRun()
                   << ", "
                   << cet::canonical_string(it->second)
                   << " ], ";
        }
        // Rewind over last delimiter.
        srstring.seekp(-2, ios_base::cur);
        srstring << " ]";
        insert_md_row(stmt, { "runs", srstring.str() });
      }
    }
    // Number of events.
    insert_md_row(stmt, { "event_count", std::to_string(stats.eventsThisFile()) });
    // first_event and last_event.
    auto eidToTuple = [](EventID const & eid) -> std::string
    {
      std::ostringstream eidStr;
      eidStr << "[ " << eid.run() << ", " << eid.subRun()
      << ", " << eid.event() << " ]";
      return eidStr.str();
    };
    insert_md_row(stmt, { "first_event",
          eidToTuple(stats.lowestEventID()) });
    insert_md_row(stmt, { "last_event",
          eidToTuple(stats.highestEventID()) });
    // File parents.
    if (!stats.parents().empty()) {
      std::ostringstream pstring;
      pstring << "[ ";
      for (auto const & parent : stats.parents()) {
        pstring << cet::canonical_string(parent) << ", ";
      }
      // Rewind over last delimiter.
      pstring.seekp(-2, ios_base::cur);
      pstring << " ]";
      insert_md_row(stmt, { "parents", pstring.str() });
    }
    ////////////////////////////////////

    // Incoming stream-specific metadata overrides.
    for ( auto const & kv : ssmd ) {
      insert_md_row(stmt, kv);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(metaDataHandle_, "END TRANSACTION;", 0, 0, SQLErrMsg());
  }

  void RootOutputFile::writeParameterSetRegistry() {
    fhicl::ParameterSetRegistry::exportTo(metaDataHandle_);
  }

  void RootOutputFile::writeProductDescriptionRegistry() {
    // Make a local copy of the MasterProductRegistry's ProductList,
    // removing any transient or pruned products.
    ProductRegistry plh(art::ProductMetaData::instance().productList());
    ProductList &pList = plh.productList_;
    set<BranchID>::iterator end = branchesWithStoredHistory_.end();
    for (ProductList::iterator
           it = pList.begin(),
           ple = pList.end();
         it != ple; ) {
      if (branchesWithStoredHistory_.find(it->second.branchID()) == end) {
        ProductList::iterator itCopy(it);
        ++it;
        pList.erase(itCopy);
      } else {
        ++it;
      }
    }

    ProductRegistry * pplh = &plh;
    TBranch* b = metaDataTree_->Branch(metaBranchRootName<ProductRegistry>(),
                                       &pplh,
                                       om_->basketSize(), 0);
    assert(b);
    b->Fill();
  }

  void RootOutputFile::writeProductDependencies() {
    BranchChildren& pDeps = const_cast<BranchChildren&>(om_->branchChildren());
    BranchChildren * ppDeps = &pDeps;
    TBranch* b = metaDataTree_->Branch(metaBranchRootName<BranchChildren>(), &ppDeps, om_->basketSize(), 0);
    assert(b);
    b->Fill();
  }

  void RootOutputFile::finishEndFile() {
    metaDataTree_->SetEntries(-1);
    RootOutputTree::writeTTree(metaDataTree_);

    RootOutputTree::writeTTree(parentageTree_);

    // Write out the tree corresponding to each BranchType
    for (int i = InEvent; i < NumBranchTypes; ++i) {
      BranchType branchType = static_cast<BranchType>(i);
      treePointers_[branchType]->writeTree();
    }

    // Write out the metadata DB
    metaDataHandle_.reset();

    // close the file -- mfp
    filePtr_->Close();
    filePtr_.reset();

    // report that file has been closed

  }

  void
  RootOutputFile::insertAncestors(ProductProvenance const& iGetParents,
                                  Principal const& principal,
                                  set<ProductProvenance>& oToFill) {
    if(om_->dropMetaData() == RootOutput::DropAll) return;
    if(om_->dropMetaDataForDroppedData()) return;
    BranchMapper const& iMapper = principal.branchMapper();
    vector<BranchID> const& parentIDs = iGetParents.parentage().parents();
    for(vector<BranchID>::const_iterator it=parentIDs.begin(), itEnd = parentIDs.end();
          it != itEnd; ++it) {
      branchesWithStoredHistory_.insert(*it);
      cet::exempt_ptr<ProductProvenance const> info = iMapper.branchToProductProvenance(*it);
      if(info && om_->dropMetaData() == RootOutput::DropNone) {
        BranchDescription const * bd(principal.getForOutput(info->branchID(),false).desc());
        if(bd && bd->produced() && oToFill.insert(*info).second) {
          //haven't seen this one yet
          insertAncestors(*info, principal, oToFill);
        }
      }
    }
  }

  void RootOutputFile::fillBranches(
                BranchType const& branchType,
                Principal const& principal,
                vector<ProductProvenance>* productProvenanceVecPtr) {

    vector<std::shared_ptr<EDProduct> > dummies;

    bool const fastCloning = (branchType == InEvent) && currentlyFastCloning_;

    OutputItemList const& items = om_->selectedOutputItemList()[branchType];

    set<ProductProvenance> provenanceToKeep;

    // Loop over EDProduct branches, fill the provenance, and write the branch.
    for (OutputItemList::const_iterator i = items.begin(), iEnd = items.end(); i != iEnd; ++i) {

      BranchID const& id = i->branchDescription_->branchID();
      branchesWithStoredHistory_.insert(id);

      bool produced = i->branchDescription_->produced();
      bool keepProvenance = om_->dropMetaData() == RootOutput::DropNone ||
                           (om_->dropMetaData() == RootOutput::DropPrior && produced);
      bool getProd = (produced || !fastCloning ||
         treePointers_[branchType]->uncloned(i->branchDescription_->branchName()));

      EDProduct const* product = 0;
      OutputHandle const oh = principal.getForOutput(id, getProd);
      if (!oh.productProvenance()) {
        // No product with this ID is in the event.
        // Create and write the provenance.
        if (keepProvenance) {
          if (produced) {
            provenanceToKeep.insert(ProductProvenance(i->branchDescription_->branchID(),
                        productstatus::neverCreated()));
          } else {
            provenanceToKeep.insert(ProductProvenance(i->branchDescription_->branchID(),
                        productstatus::dropped()));
          }
        }
      } else {
        product = oh.wrapper();
        if (keepProvenance) {
          provenanceToKeep.insert(*oh.productProvenance());
          insertAncestors(*oh.productProvenance(), principal, provenanceToKeep);
        }
      }
      if (getProd) {
        if (product == 0) {
          // No product with this ID is in the event.
          // Add a null product.
          TClass *cp = TClass::GetClass(i->branchDescription_->wrappedName().c_str());
          std::shared_ptr<EDProduct> dummy(static_cast<EDProduct *>(cp->New()));
          dummies.push_back(dummy);
          product = dummy.get();
        }
        i->product_ = product;
      }
    }

    productProvenanceVecPtr->assign(provenanceToKeep.begin(), provenanceToKeep.end());
    treePointers_[branchType]->fillTree();
    productProvenanceVecPtr->clear();
  }

}  // art
