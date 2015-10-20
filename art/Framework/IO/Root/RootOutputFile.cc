#include "art/Framework/IO/Root/RootOutputFile.h"
// vim: set sw=2:

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/IO/FileStatsCollector.h"
#include "art/Framework/IO/Root/DropMetaData.h"
#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/GetFileFormatVersion.h"
#include "art/Framework/IO/Root/rootNames.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
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
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/ResultsAuxiliary.h"
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
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include "Rtypes.h"
#include "TBranchElement.h"
#include "TClass.h"
#include "TFile.h"
#include "TTree.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>
#include <utility>
#include <vector>

using namespace cet;
using namespace std;
using art::rootNames::metaBranchRootName;
using art::RootOutputFile;

namespace {

  void
  insert_md_row(sqlite3_stmt* stmt, pair<string, string> const& kv)
  {
    string const& theName(kv.first);
    string const& theValue(kv.second);
    sqlite3_bind_text(stmt, 1, theName.c_str(),
                      theName.size() + 1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, theValue.c_str(),
                      theValue.size() + 1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_reset(stmt);
  }

} // unnamed namespace

art::
RootOutputFile::
RootOutputFile(OutputModule* om, string const& fileName,
               unsigned int const maxFileSize, int const compressionLevel,
               int64_t const saveMemoryObjectThreshold,
               int64_t const treeMaxVirtualSize,
               int const splitLevel, int const basketSize,
               DropMetaData dropMetaData, bool dropMetaDataForDroppedData,
               bool fastCloning)
  : file_(fileName)
  , om_(om)
  , maxFileSize_(maxFileSize)
  , compressionLevel_(compressionLevel)
  , saveMemoryObjectThreshold_(saveMemoryObjectThreshold)
  , treeMaxVirtualSize_(treeMaxVirtualSize)
  , splitLevel_(splitLevel)
  , basketSize_(basketSize)
  , dropMetaData_(dropMetaData)
  , dropMetaDataForDroppedData_(dropMetaDataForDroppedData)
  , fastCloning_(fastCloning)
  , currentlyFastCloning_(true)
  , filePtr_(TFile::Open(file_.c_str(), "recreate", "",
                         compressionLevel))
  , fileIndex_()
  , eventEntryNumber_(0LL)
  , subRunEntryNumber_(0LL)
  , runEntryNumber_(0LL)
  , metaDataTree_(nullptr)
  , fileIndexTree_(nullptr)
  , parentageTree_(nullptr)
  , eventHistoryTree_(nullptr)
  , pEventAux_(nullptr)
  , pSubRunAux_(nullptr)
  , pRunAux_(nullptr)
  , pResultsAux_(nullptr)
  , eventProductProvenanceVector_()
  , subRunProductProvenanceVector_()
  , runProductProvenanceVector_()
  , resultsProductProvenanceVector_()
  , pEventProductProvenanceVector_(&eventProductProvenanceVector_)
  , pSubRunProductProvenanceVector_(&subRunProductProvenanceVector_)
  , pRunProductProvenanceVector_(&runProductProvenanceVector_)
  , pResultsProductProvenanceVector_(&resultsProductProvenanceVector_)
  , pHistory_(nullptr)
  , treePointers_ { // Order (and number) must match BranchTypes.h!
  std::make_unique<RootOutputTree>(static_cast<EventPrincipal*>(nullptr),
                                   filePtr_, InEvent, pEventAux_,
                                   pEventProductProvenanceVector_, basketSize, splitLevel,
                                   treeMaxVirtualSize, saveMemoryObjectThreshold),
    std::make_unique<RootOutputTree>(static_cast<SubRunPrincipal*>(nullptr),
                                     filePtr_, InSubRun, pSubRunAux_,
                                     pSubRunProductProvenanceVector_, basketSize, splitLevel,
                                     treeMaxVirtualSize, saveMemoryObjectThreshold),
    std::make_unique<RootOutputTree>(static_cast<RunPrincipal*>(nullptr),
                                     filePtr_, InRun, pRunAux_,
                                     pRunProductProvenanceVector_, basketSize, splitLevel,
                                     treeMaxVirtualSize, saveMemoryObjectThreshold),
    std::make_unique<RootOutputTree>(static_cast<ResultsPrincipal*>(nullptr),
                                     filePtr_, InResults, pResultsAux_,
                                     pResultsProductProvenanceVector_, basketSize, splitLevel,
                                     treeMaxVirtualSize, saveMemoryObjectThreshold) }
  , dataTypeReported_(false)
  , metaDataHandle_(filePtr_.get(), "RootFileDB",
                    SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE)
  , selectedOutputItemList_()
{
  // Don't split metadata tree or event description tree
  metaDataTree_ = RootOutputTree::makeTTree(filePtr_.get(),
                                            rootNames::metaDataTreeName(), 0);
  fileIndexTree_ = RootOutputTree::makeTTree(filePtr_.get(),
                                             rootNames::fileIndexTreeName(), 0);
  parentageTree_ = RootOutputTree::makeTTree(filePtr_.get(),
                                             rootNames::parentageTreeName(), 0);
  // Create the tree that will carry (event) History objects.
  eventHistoryTree_ = RootOutputTree::makeTTree(filePtr_.get(),
                                                rootNames::eventHistoryTreeName(), splitLevel);
  if (!eventHistoryTree_) {
    throw art::Exception(art::errors::FatalRootError)
      << "Failed to create the tree for History objects\n";
  }
  pHistory_ = new History;
  if (!eventHistoryTree_->Branch(rootNames::eventHistoryBranchName().c_str(),
                                 &pHistory_, basketSize, 0)) {
    throw art::Exception(art::errors::FatalRootError)
      << "Failed to create a branch for History in the output file\n";
  }
  delete pHistory_;
  pHistory_ = nullptr;
}

art::
RootOutputFile::OutputItem::Sorter::
Sorter(TTree* tree)
{
  // Fill a map mapping branch names to an index specifying
  // the order in the tree.
  if (!tree) {
    return;
  }
  auto branches = tree->GetListOfBranches();
  for (int i = 0, sz = branches->GetEntries(); i != sz; ++i) {
    auto br = reinterpret_cast<TBranchElement*>(branches->At(i));
    treeMap_.emplace(string(br->GetName()), i);
  }
}

bool
art::
RootOutputFile::OutputItem::Sorter::
operator()(OutputItem const& lh, OutputItem const& rh) const
{
  // Provides a comparison for sorting branches according
  // to the index values in treeMap_.  Branches not found
  // are always put at the end, i.e. <not-found> is greater
  // than <found>.
  if (treeMap_.empty()) {
    return lh < rh;
  }
  auto const& lname = lh.branchDescription_->branchName();
  auto const& rname = rh.branchDescription_->branchName();
  auto lit = treeMap_.find(lname);
  auto rit = treeMap_.find(rname);
  bool lfound = (lit != treeMap_.end());
  bool rfound = (rit != treeMap_.end());
  if (lfound && rfound) {
    return lit->second < rit->second;
  }
  if (lfound) {
    return true;
  }
  if (rfound) {
    return false;
  }
  return lh < rh;
}

void
art::
RootOutputFile::
selectProducts(FileBlock const& fb)
{
  for (int i = InEvent; i < NumBranchTypes; ++i) {
    auto bt = static_cast<BranchType>(i);
    auto& items = selectedOutputItemList_[bt];
    for (auto const& val : items) {
      treePointers_[bt]->resetOutputBranchAddress(*val.branchDescription_);
    }
    items.clear();
    for (auto const& bd : om_->keptProducts()[bt]) {
      if ((bt < InResults) || bd->produced()) {
        items.emplace_back(bd);
      }
    }
    if ((bt == InEvent) && (fb.tree() != nullptr)) {
      // Only care about sorting event tree because that's the only one
      // we might fast clone.
      sort(items.begin(), items.end(), OutputItem::Sorter(fb.tree()));
    }
    for (auto const& val : items) {
      treePointers_[bt]->addOutputBranch(*val.branchDescription_, val.product_);
    }
  }
}

void
art::
RootOutputFile::
beginInputFile(FileBlock const& fb, bool fastClone)
{
  selectProducts(fb);
  auto const origCurrentlyFastCloning = currentlyFastCloning_;
  currentlyFastCloning_ = fastCloning_ && fastClone;
  if (currentlyFastCloning_ &&
      !treePointers_[InEvent]->checkSplitLevelAndBasketSize(fb.tree())) {
    mf::LogWarning("FastCloning")
      << "Fast cloning deactivated for this input file due to "
      << "splitting level and/or basket size.";
    currentlyFastCloning_ = false;
  }
  if (currentlyFastCloning_ && !origCurrentlyFastCloning) {
    mf::LogWarning("FastCloning")
      << "Fast cloning reactivated for this input file.";
  }
  treePointers_[InEvent]->beginInputFile(currentlyFastCloning_);
  treePointers_[InEvent]->fastCloneTree(fb.tree());
}

void
art::
RootOutputFile::
respondToCloseInputFile(FileBlock const&)
{
  for (auto const & treePtr : treePointers_) {
    treePtr->setEntries();
  }
}

bool
art::
RootOutputFile::
shouldWeCloseFile() const
{
  unsigned int const oneK = 1024;
  Long64_t size = filePtr_->GetSize() / oneK;
  return size >= maxFileSize_;
}

void
art::
RootOutputFile::
writeOne(EventPrincipal const& e)
{
  // Auxiliary branch.
  // Note: pEventAux_ must be set before calling fillBranches
  // since it gets written out in that routine.
  pEventAux_ = &e.aux();
  // Because getting the data may cause an exception to be
  // thrown we want to do that first before writing anything
  // to the file about this event.
  fillBranches(InEvent, e, pEventProductProvenanceVector_);
  // History branch.
  History historyForOutput(e.history());
  historyForOutput.addEventSelectionEntry(om_->selectorConfig());
  pHistory_ = &historyForOutput;
  int sz = eventHistoryTree_->Fill();
  if (sz <= 0) {
    throw art::Exception(art::errors::FatalRootError)
      << "Failed to fill the History tree for event: "
      << e.id()
      << "\nTTree::Fill() returned "
      << sz
      << " bytes written."
      << endl;
  }
  // Add the dataType to the job report if it hasn't already been done
  if (!dataTypeReported_) {
    string dataType("MC");
    if (pEventAux_->isRealData()) {
      dataType = "Data";
    }
    dataTypeReported_ = true;
  }
  pHistory_ = &e.history();
  // Add event to index
  fileIndex_.addEntry(pEventAux_->id(), eventEntryNumber_);
  ++eventEntryNumber_;
}

void
art::
RootOutputFile::
writeSubRun(SubRunPrincipal const& sr)
{
  // Auxiliary branch
  pSubRunAux_ = &sr.aux();
  // Add subRun to index.
  fileIndex_.addEntry(EventID::invalidEvent(pSubRunAux_->id()),
                      subRunEntryNumber_);
  ++subRunEntryNumber_;
  fillBranches(InSubRun, sr, pSubRunProductProvenanceVector_);
}

void
art::
RootOutputFile::
writeRun(RunPrincipal const& r)
{
  // Auxiliary branch
  pRunAux_ = &r.aux();
  // Add run to index.
  fileIndex_.addEntry(EventID::invalidEvent(pRunAux_->id()), runEntryNumber_);
  ++runEntryNumber_;
  fillBranches(InRun, r, pRunProductProvenanceVector_);
}

void
art::
RootOutputFile::
writeParentageRegistry()
{
  ParentageID const* hash = new ParentageID;
  if (!parentageTree_->Branch(rootNames::parentageIDBranchName().c_str(),
                              &hash, basketSize_, 0)) {
    throw art::Exception(art::errors::FatalRootError)
      << "Failed to create a branch for ParentageIDs in the output file";
  }
  delete hash;
  hash = nullptr;
  Parentage const* desc = new Parentage;
  if (!parentageTree_->Branch(rootNames::parentageBranchName().c_str(), &desc,
                              basketSize_, 0)) {
    throw art::Exception(art::errors::FatalRootError)
      << "Failed to create a branch for Parentages in the output file";
  }
  delete desc;
  desc = nullptr;
  //map<art::ParentID const, art::Parentage>
  //map<art::Hash<ParentageType> const, art::Parentage>
  //map<art::Hash<5> const, art::Parentage>
  for (auto I = ParentageRegistry::cbegin(), E = ParentageRegistry::cend();
       I != E; ++I) {
    hash = &I->first;
    desc = &I->second;
    parentageTree_->Fill();
  }
  parentageTree_->SetBranchAddress(rootNames::parentageIDBranchName().c_str(),
                                   0);
  parentageTree_->SetBranchAddress(rootNames::parentageBranchName().c_str(),
                                   0);
}

void
art::
RootOutputFile::
writeFileFormatVersion()
{
  FileFormatVersion ver(getFileFormatVersion(), getFileFormatEra());
  FileFormatVersion* pver = &ver;
  TBranch* b = metaDataTree_->Branch(metaBranchRootName<FileFormatVersion>(),
                                     &pver, basketSize_, 0);
  // FIXME: Turn this into a throw!
  assert(b);
  b->Fill();
}

void
art::
RootOutputFile::
writeFileIndex()
{
  fileIndex_.sortBy_Run_SubRun_Event();
  FileIndex::Element const* findexElemPtr = nullptr;
  TBranch* b = fileIndexTree_->Branch(metaBranchRootName<FileIndex::Element>(),
                                      &findexElemPtr, basketSize_, 0);
  // FIXME: Turn this into a throw!
  assert(b);
  for (auto& entry: fileIndex_) {
    findexElemPtr = &entry;
    b->Fill();
  }
}

void
art::
RootOutputFile::
writeEventHistory()
{
  RootOutputTree::writeTTree(eventHistoryTree_);
}

void
art::
RootOutputFile::
writeProcessConfigurationRegistry()
{
  // We don't do this yet; currently we're storing a slightly
  // bloated ProcessHistoryRegistry.
}

void
art::
RootOutputFile::
writeProcessHistoryRegistry()
{
  ProcessHistoryMap const& r = ProcessHistoryRegistry::get();
  ProcessHistoryMap* p = &const_cast<ProcessHistoryMap&>(r);
  TBranch* b = metaDataTree_->Branch(metaBranchRootName<ProcessHistoryMap>(),
                                     &p, basketSize_, 0);
  if (b != nullptr) {
    b->Fill();
  } else {
    throw Exception(errors::LogicError)
      << "Unable to locate required ProcessHistoryMap branch in output metadata tree.\n";
  }
}

void
art::
RootOutputFile::
writeBranchIDListRegistry()
{
  BranchIDLists* p = &BranchIDListRegistry::instance()->data();
  TBranch* b = metaDataTree_->Branch(metaBranchRootName<BranchIDLists>(), &p,
                                     basketSize_, 0);
  // FIXME: Turn this into a throw!
  assert(b);
  b->Fill();
}

void
art::
RootOutputFile::
writeFileCatalogMetadata(FileStatsCollector const& stats,
                         FileCatalogMetadata::collection_type const& md,
                         FileCatalogMetadata::collection_type const& ssmd)
{
  SQLErrMsg errMsg;
  // ID is declared auto-increment, so don't specify it when filling a
  // row.
  sqlite3_exec(metaDataHandle_,
               "BEGIN TRANSACTION; "
               "DROP TABLE IF EXISTS FileCatalog_metadata; "
               "CREATE TABLE FileCatalog_metadata(ID INTEGER PRIMARY KEY,"
               "                                  Name, Value); "
               "COMMIT;", 0, 0, errMsg);
  errMsg.throwIfError();
  sqlite3_exec(metaDataHandle_, "BEGIN TRANSACTION;", 0, 0, errMsg);
  sqlite3_stmt* stmt = 0;
  sqlite3_prepare_v2(metaDataHandle_,
                     "INSERT INTO FileCatalog_metadata(Name, Value) "
                     "VALUES(?, ?);", -1, &stmt, NULL);
  for (auto const& kv : md) {
    insert_md_row(stmt, kv);
  }
  // Add our own specific information: File format and friends.
  insert_md_row(stmt, { "file_format", "\"artroot\"" });
  insert_md_row(stmt, { "file_format_era",
        cet::canonical_string(getFileFormatEra()) });
  insert_md_row(stmt, { "file_format_version",
        to_string(getFileFormatVersion())
        });
  namespace bpt = boost::posix_time;
  // File start time.
  insert_md_row(stmt, { "start_time", cet::canonical_string(
                                                            bpt::to_iso_extended_string(
                                                                                        stats.outputFileOpenTime()))
        });
  // File "end" time: now, since file is not actually closed yet.
  insert_md_row(stmt, { "end_time", cet::canonical_string(
                                                          bpt::to_iso_extended_string(
                                                                                      boost::posix_time::second_clock::universal_time()))
        });
  // Run/subRun information.
  if (!stats.seenSubRuns().empty()) {

    auto I = find_if(md.crbegin(), md.crend(),
                     [](auto const& p){ return p.first == "run_type"; } );

    if (I != md.crend()) {
      ostringstream buf;
      buf << "[ ";
      bool first = true;
      for (auto const& srid : stats.seenSubRuns()) {
        if (first) {
          first = false;
        }
        else {
          buf << ", ";
        }
        buf << "[ "
            << srid.run()
            << ", "
            << srid.subRun()
            << ", "
            << cet::canonical_string(I->second)
            << " ], ";
      }
      // Rewind over last delimiter.
      buf.seekp(-2, ios_base::cur);
      buf << " ]";
      insert_md_row(stmt, { "runs", buf.str() });
    }
  }
  // Number of events.
  insert_md_row(stmt, { "event_count", to_string(stats.eventsThisFile()) });
  // first_event and last_event.
  auto eidToTuple = [](EventID const & eid)->string {
    ostringstream eidStr;
    eidStr << "[ "
    << eid.run()
    << ", "
    << eid.subRun()
    << ", "
    << eid.event()
    << " ]";
    return eidStr.str();
  };
  insert_md_row(stmt, { "first_event", eidToTuple(stats.lowestEventID()) });
  insert_md_row(stmt, { "last_event", eidToTuple(stats.highestEventID()) });
  // File parents.
  if (!stats.parents().empty()) {
    ostringstream pstring;
    pstring << "[ ";
    for (auto const& parent : stats.parents()) {
      pstring << cet::canonical_string(parent) << ", ";
    }
    // Rewind over last delimiter.
    pstring.seekp(-2, ios_base::cur);
    pstring << " ]";
    insert_md_row(stmt, { "parents", pstring.str() });
  }
  // Incoming stream-specific metadata overrides.
  for (auto const& kv : ssmd) {
    insert_md_row(stmt, kv);
  }
  sqlite3_finalize(stmt);
  sqlite3_exec(metaDataHandle_, "END TRANSACTION;", 0, 0, SQLErrMsg());
}

void
art::
RootOutputFile::
writeParameterSetRegistry()
{
  fhicl::ParameterSetRegistry::exportTo(metaDataHandle_);
}

void
art::
RootOutputFile::
writeProductDescriptionRegistry()
{
  // Make a local copy of the MasterProductRegistry's ProductList,
  // removing any transient or pruned products.
  auto end = branchesWithStoredHistory_.end();

  ProductRegistry reg;
  for ( auto const& pr : ProductMetaData::instance().productList() ) {
    if ( branchesWithStoredHistory_.find(pr.second.branchID()) == end ){
      continue;
    }
    reg.productList_.emplace_hint(reg.productList_.end(),pr);
  }

  auto* regp = &reg;
  TBranch* b = metaDataTree_->Branch(metaBranchRootName<ProductRegistry>(),
                                     &regp, basketSize_, 0);
  // FIXME: Turn this into a throw!
  assert(b);
  b->Fill();
}

void
art::
RootOutputFile::
writeProductDependencies()
{
  BranchChildren& pDeps = const_cast<BranchChildren&>(om_->branchChildren());
  BranchChildren* ppDeps = &pDeps;
  TBranch* b = metaDataTree_->Branch(metaBranchRootName<BranchChildren>(),
                                     &ppDeps, basketSize_, 0);
  // FIXME: Turn this into a throw!
  assert(b);
  b->Fill();
}

void
art::
RootOutputFile::
writeResults(ResultsPrincipal & resp)
{
  pResultsAux_ = &resp.aux();
  fillBranches(InResults, resp, pResultsProductProvenanceVector_);
}

void
RootOutputFile::
finishEndFile()
{
  metaDataTree_->SetEntries(-1);
  RootOutputTree::writeTTree(metaDataTree_);
  RootOutputTree::writeTTree(fileIndexTree_);
  RootOutputTree::writeTTree(parentageTree_);
  // Write out the tree corresponding to each BranchType
  for (int i = InEvent; i < NumBranchTypes; ++i) {
    BranchType branchType = static_cast<BranchType>(i);
    treePointers_[branchType]->writeTree();
  }
  // Write out the metadata DB
  metaDataHandle_.reset();
  // Close the file.
  filePtr_->Close();
  filePtr_.reset();
}

void
art::
RootOutputFile::
insertAncestors(ProductProvenance const& iGetParents,
                Principal const& principal, set<ProductProvenance>& oToFill)
{
  if (dropMetaData_ == DropMetaData::DropAll) {
    return;
  }
  if (dropMetaDataForDroppedData_) {
    return;
  }
  auto parentIDs = iGetParents.parentage().parents();
  for (auto I = parentIDs.cbegin(), E = parentIDs.cend(); I != E; ++I) {
    branchesWithStoredHistory_.insert(*I);
    auto info = principal.branchMapper().branchToProductProvenance(*I);
    if (!info || dropMetaData_ != DropMetaData::DropNone) {
      continue;
    }
    auto bd = principal.getForOutput(info->branchID(), false).desc();
    if (bd && bd->produced() && oToFill.insert(*info).second) {
      // FIXME: Remove recursion!
      insertAncestors(*info, principal, oToFill);
    }
  }
}

void
art::
RootOutputFile::
fillBranches(BranchType const& bt, Principal const& principal,
             vector<ProductProvenance>* vpp)
{
  vector<unique_ptr<EDProduct>> dummies;
  bool const fastCloning = (bt == InEvent) && currentlyFastCloning_;
  set<ProductProvenance> keptProv;
  for (auto const& val : selectedOutputItemList_[bt]) {
    auto const& bd = val.branchDescription_;
    auto const& bid = bd->branchID();
    branchesWithStoredHistory_.insert(bid);
    auto produced = bd->produced();
    bool keepProvenance = dropMetaData_ == DropMetaData::DropNone ||
      (dropMetaData_ == DropMetaData::DropPrior &&
       produced);
    bool resolveProd = (produced || !fastCloning ||
                        treePointers_[bt]->uncloned(bd->branchName()));
    auto const oh = principal.getForOutput(bid, resolveProd);
    EDProduct const* product = nullptr;
    if (oh.productProvenance()) {
      product = oh.wrapper();
    }
    if (keepProvenance) {
      if (oh.productProvenance()) {
        keptProv.insert(*oh.productProvenance());
        insertAncestors(*oh.productProvenance(), principal, keptProv);
      }
      else {
        // No provenance, product was either not produced,
        // or was dropped, create provenance to remember that.
        if (produced) {
          keptProv.emplace(bd->branchID(), productstatus::neverCreated());
        }
        else {
          keptProv.emplace(bd->branchID(), productstatus::dropped());
        }
      }
    }
    if (resolveProd) {
      if (product == nullptr) {
        // No such product in the event, so use a dummy product.
        // FIXME: Can we cache these dummy products so that we do not
        // FIXME: create them for every event?
        auto name = bd->wrappedName().c_str();
        TClass* cp = TClass::GetClass(name);
        if (cp == nullptr) {
          throw art::Exception(art::errors::DictionaryNotFound)
            << "TClass::GetClass() returned null pointer for name: "
            << name
            << '\n';
        }
        unique_ptr<EDProduct> dummy(reinterpret_cast<EDProduct*>(cp->New()));
        product = dummy.get();
        // Make sure the dummies outlive the fillTree call.
        dummies.emplace_back(move(dummy));
      }
      val.product_ = product;
    }
  }
  vpp->assign(keptProv.begin(), keptProv.end());
  treePointers_[bt]->fillTree();
  vpp->clear();
}

