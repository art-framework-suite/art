#include "art/Framework/IO/Root/RootOutputFile.h"
// vim: set sw=2:

#include "Rtypes.h"
#include "TBranchElement.h"
#include "TClass.h"
#include "TFile.h"
#include "TTree.h"
#include "art/Framework/IO/FileStatsCollector.h"
#include "art/Framework/IO/Root/DropMetaData.h"
#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/GetFileFormatVersion.h"
#include "art/Framework/IO/Root/RootDB/SQLErrMsg.h"
#include "art/Framework/IO/Root/RootDB/TKeyVFSOpenPolicy.h"
#include "art/Framework/IO/Root/RootFileBlock.h"
#include "art/Framework/IO/Root/checkDictionaries.h"
#include "art/Framework/IO/Root/detail/KeptProvenance.h"
#include "art/Framework/IO/Root/detail/getObjectRequireDict.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Services/System/DatabaseConnection.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Version/GetReleaseVersion.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "canvas/Persistency/Provenance/BranchChildren.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ParameterSetBlob.h"
#include "canvas/Persistency/Provenance/Parentage.h"
#include "canvas/Persistency/Provenance/ParentageRegistry.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Persistency/Provenance/ResultsAuxiliary.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Utilities/Exception.h"
#include "canvas_root_io/Utilities/DictionaryChecker.h"
#include "cetlib/canonical_string.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/sqlite/Ntuple.h"
#include "cetlib/sqlite/Transaction.h"
#include "cetlib/sqlite/create_table.h"
#include "cetlib/sqlite/exec.h"
#include "cetlib/sqlite/insert.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <algorithm>
#include <utility>
#include <vector>

using namespace cet;
using namespace std;
using art::BranchType;
using art::RootOutputFile;
using art::rootNames::metaBranchRootName;

namespace {

  void
  create_table(sqlite3* const db,
               std::string const& name,
               std::vector<std::string> const& columns,
               std::string const& suffix = {})
  {
    if (columns.empty())
      throw art::Exception(art::errors::LogicError)
        << "Number of sqlite columns specified for table: " << name << '\n'
        << "is zero.\n";

    std::string ddl = "DROP TABLE IF EXISTS " + name +
                      "; "
                      "CREATE TABLE " +
                      name + "(" + columns.front();
    std::for_each(columns.begin() + 1, columns.end(), [&ddl](auto const& col) {
      ddl += "," + col;
    });
    ddl += ") ";
    ddl += suffix;
    ddl += ";";
    sqlite::exec(db, ddl);
  }

  void
  insert_eventRanges_row(sqlite3_stmt* stmt,
                         art::SubRunNumber_t const sr,
                         art::EventNumber_t const b,
                         art::EventNumber_t const e)
  {
    sqlite3_bind_int64(stmt, 1, sr);
    sqlite3_bind_int64(stmt, 2, b);
    sqlite3_bind_int64(stmt, 3, e);
    sqlite3_step(stmt);
    sqlite3_reset(stmt);
  }

  void
  insert_rangeSets_eventSets_row(sqlite3_stmt* stmt,
                                 unsigned const rsid,
                                 unsigned const esid)
  {
    sqlite3_bind_int64(stmt, 1, rsid);
    sqlite3_bind_int64(stmt, 2, esid);
    sqlite3_step(stmt);
    sqlite3_reset(stmt);
  }

  unsigned
  getNewRangeSetID(sqlite3* db,
                   art::BranchType const bt,
                   art::RunNumber_t const r)
  {
    sqlite::insert_into(db, art::BranchTypeToString(bt) + "RangeSets")
      .values(r);
    return sqlite3_last_insert_rowid(db);
  }

  vector<unsigned>
  getExistingRangeSetIDs(sqlite3* db, art::RangeSet const& rs)
  {
    vector<unsigned> rangeSetIDs;
    cet::transform_all(
      rs, std::back_inserter(rangeSetIDs), [db](auto const& range) {
        sqlite::query_result<unsigned> r;
        r << sqlite::select("ROWID")
               .from(db, "EventRanges")
               .where("SubRun=" + std::to_string(range.subRun()) +
                      " AND "
                      "begin=" +
                      std::to_string(range.begin()) +
                      " AND "
                      "end=" +
                      std::to_string(range.end()));
        return unique_value(r);
      });
    return rangeSetIDs;
  }

  void
  insertIntoEventRanges(sqlite3* db, art::RangeSet const& rs)
  {
    sqlite::Transaction txn{db};
    sqlite3_stmt* stmt{nullptr};
    std::string const ddl{"INSERT INTO EventRanges(SubRun, begin, end) "
                          "VALUES(?, ?, ?);"};
    sqlite3_prepare_v2(db, ddl.c_str(), -1, &stmt, nullptr);
    for (auto const& range : rs) {
      insert_eventRanges_row(stmt, range.subRun(), range.begin(), range.end());
    }
    sqlite3_finalize(stmt);
    txn.commit();
  }

  void
  insertIntoJoinTable(sqlite3* db,
                      art::BranchType const bt,
                      unsigned const rsID,
                      vector<unsigned> const& eventRangesIDs)
  {
    sqlite::Transaction txn{db};
    sqlite3_stmt* stmt{nullptr};
    std::string const ddl{
      "INSERT INTO " + art::BranchTypeToString(bt) +
      "RangeSets_EventRanges(RangeSetsID, EventRangesID) Values(?,?);"};
    sqlite3_prepare_v2(db, ddl.c_str(), -1, &stmt, nullptr);
    cet::for_all(eventRangesIDs, [stmt, rsID](auto const eventRangeID) {
      insert_rangeSets_eventSets_row(stmt, rsID, eventRangeID);
    });
    sqlite3_finalize(stmt);
    txn.commit();
  }

  void
  maybeInvalidateRangeSet(BranchType const bt,
                          art::RangeSet const& principalRS,
                          art::RangeSet& productRS)
  {
    if (!productRS.is_valid())
      return;

    assert(principalRS.is_sorted());
    assert(productRS.is_sorted());

    if (bt == art::InRun && productRS.is_full_run())
      return;
    if (bt == art::InSubRun && productRS.is_full_subRun())
      return;
    assert(!productRS.ranges().empty());

    auto const r = productRS.run();
    auto const& productFront = productRS.ranges().front();
    if (!principalRS.contains(r, productFront.subRun(), productFront.begin()))
      productRS = art::RangeSet::invalid();
  }

  using art::detail::RangeSetsSupported;

  // The purpose of 'maybeInvalidateRangeSet' is to support the
  // following situation.  Suppose process 1 creates three files with
  // one Run product each, all corresponding to the same Run.  Let's
  // call the individual Run product instances in the three separate
  // files as A, B, and C.  Now suppose that the three files serve as
  // inputs to process 2, where a concatenation is being performed AND
  // ALSO an output file switch.  Process 2 results in two output
  // files, and now, in process 3, we concatenate the outputs from
  // process 2.  The situation would look like this:
  //
  //  Process 1:   [A]     [B]     [C]
  //                 \     / \     /
  //  Process 2:     [A + B] [B + C]
  //                   \ /     \ /
  //        D=agg(A,B)  |       |  E=agg(B,C)
  //                     \     /
  //  Process 3:         [D + E]
  //
  // Notice the complication in process 3: product 'B' will be
  // aggregated twice: once with A, and once with C.  Whenever the
  // output from process 3 is read as input to another process, the
  // fetched product will be equivalent to A+2B+C.
  //
  // To avoid this situation, we compare the RangeSet of the product
  // with the RangeSet of the in-memory RunAuxiliary.  If the
  // beginning of B's RangeSet is not contained within the auxiliary's
  // RangeSet, then a dummy product with an invalid RangeSet is
  // written to disk.  Instead of the diagram above, we have:
  //
  //  Process 1:   [A]     [B]     [C]
  //                 \     / \     /
  //  Process 2:     [A + B] [x + C]
  //                   \ /     \ /
  //        D=agg(A,B)  |       |  E=agg(x,C)=C
  //                     \     /
  //  Process 3:         [D + E]
  //
  // where 'x' represent a dummy product.  Upon aggregating D and E,
  // we obtain the correctly formed A+B+C product.
  template <BranchType BT>
  std::enable_if_t<RangeSetsSupported<BT>::value, art::RangeSet>
  getRangeSet(art::OutputHandle const& oh,
              art::RangeSet const& principalRS,
              bool const producedInThisProcess)
  {
    auto rs = oh.isValid() ? oh.rangeOfValidity() : art::RangeSet::invalid();
    // Because a user can specify (e.g.):
    //   r.put(std::move(myProd), art::runFragment(myRangeSet));
    // products that are produced in this process can have valid, yet
    // arbitrary RangeSets.  We therefore never invalidate a RangeSet
    // that corresponds to a product produced in this process.
    //
    // It is possible for a user to specify a RangeSet which does not
    // correspond AT ALL to the in-memory auxiliary RangeSet.  In that
    // case, users should not expect to be able to retrieve products
    // for which no corresponding events or sub-runs were processed.
    if (!producedInThisProcess) {
      maybeInvalidateRangeSet(BT, principalRS, rs);
    }
    return rs;
  }

  template <BranchType BT>
  std::enable_if_t<!RangeSetsSupported<BT>::value, art::RangeSet>
  getRangeSet(art::OutputHandle const&,
              art::RangeSet const& /*principalRS*/,
              bool const /*producedInThisProcess*/)
  {
    return art::RangeSet::invalid();
  }

  template <BranchType BT>
  std::enable_if_t<!RangeSetsSupported<BT>::value>
  setProductRangeSetID(art::RangeSet const& /*rs*/,
                       sqlite3*,
                       art::EDProduct*,
                       std::map<unsigned, unsigned>& /*checksumToIndexLookup*/)
  {}

  template <BranchType BT>
  std::enable_if_t<RangeSetsSupported<BT>::value>
  setProductRangeSetID(art::RangeSet const& rs,
                       sqlite3* db,
                       art::EDProduct* product,
                       std::map<unsigned, unsigned>& checksumToIndexLookup)
  {
    if (!rs.is_valid()) // Invalid range-sets not written to DB
      return;

    // Set range sets for SubRun and Run products
    auto it = checksumToIndexLookup.find(rs.checksum());
    if (it != checksumToIndexLookup.cend()) {
      product->setRangeSetID(it->second);
    } else {
      unsigned const rsID = getNewRangeSetID(db, BT, rs.run());
      product->setRangeSetID(rsID);
      checksumToIndexLookup.emplace(rs.checksum(), rsID);
      insertIntoEventRanges(db, rs);
      auto const& eventRangesIDs = getExistingRangeSetIDs(db, rs);
      insertIntoJoinTable(db, BT, rsID, eventRangesIDs);
    }
  }

} // unnamed namespace

art::RootOutputFile::RootOutputFile(OutputModule* om,
                                    string const& fileName,
                                    ClosingCriteria const& fileSwitchCriteria,
                                    int const compressionLevel,
                                    int64_t const saveMemoryObjectThreshold,
                                    int64_t const treeMaxVirtualSize,
                                    int const splitLevel,
                                    int const basketSize,
                                    DropMetaData dropMetaData,
                                    bool const dropMetaDataForDroppedData,
                                    bool const fastCloningRequested)
  : om_{om}
  , file_{fileName}
  , fileSwitchCriteria_{fileSwitchCriteria}
  , compressionLevel_{compressionLevel}
  , saveMemoryObjectThreshold_{saveMemoryObjectThreshold}
  , treeMaxVirtualSize_{treeMaxVirtualSize}
  , splitLevel_{splitLevel}
  , basketSize_{basketSize}
  , dropMetaData_{dropMetaData}
  , dropMetaDataForDroppedData_{dropMetaDataForDroppedData}
  , fastCloningEnabledAtConstruction_{fastCloningRequested}
  , filePtr_{TFile::Open(file_.c_str(), "recreate", "", compressionLevel)}
  , treePointers_ { // Order (and number) must match BranchTypes.h!
    std::make_unique<RootOutputTree>(filePtr_.get(), InEvent, pEventAux_,
                                     pEventProductProvenanceVector_, basketSize, splitLevel,
                                     treeMaxVirtualSize, saveMemoryObjectThreshold),
    std::make_unique<RootOutputTree>(filePtr_.get(), InSubRun, pSubRunAux_,
                                     pSubRunProductProvenanceVector_, basketSize, splitLevel,
                                     treeMaxVirtualSize, saveMemoryObjectThreshold),
    std::make_unique<RootOutputTree>(filePtr_.get(), InRun, pRunAux_,
                                     pRunProductProvenanceVector_, basketSize, splitLevel,
                                     treeMaxVirtualSize, saveMemoryObjectThreshold),
    std::make_unique<RootOutputTree>(filePtr_.get(), InResults, pResultsAux_,
                                     pResultsProductProvenanceVector_, basketSize, splitLevel,
                                     treeMaxVirtualSize, saveMemoryObjectThreshold) }
  , rootFileDB_{ServiceHandle<DatabaseConnection>{}->get<TKeyVFSOpenPolicy>("RootFileDB",
                                                                            filePtr_.get(),
                                                                            SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)}
{
  // Don't split metadata tree or event description tree
  metaDataTree_ =
    RootOutputTree::makeTTree(filePtr_.get(), rootNames::metaDataTreeName(), 0);
  fileIndexTree_ = RootOutputTree::makeTTree(
    filePtr_.get(), rootNames::fileIndexTreeName(), 0);
  parentageTree_ = RootOutputTree::makeTTree(
    filePtr_.get(), rootNames::parentageTreeName(), 0);
  // Create the tree that will carry (event) History objects.
  eventHistoryTree_ = RootOutputTree::makeTTree(
    filePtr_.get(), rootNames::eventHistoryTreeName(), splitLevel);
  if (!eventHistoryTree_) {
    throw art::Exception(art::errors::FatalRootError)
      << "Failed to create the tree for History objects\n";
  }

  pHistory_ = new History;
  if (!eventHistoryTree_->Branch(rootNames::eventHistoryBranchName().c_str(),
                                 &pHistory_,
                                 basketSize,
                                 0)) {
    throw art::Exception(art::errors::FatalRootError)
      << "Failed to create a branch for History in the output file\n";
  }
  delete pHistory_;
  pHistory_ = nullptr;

  // Check that dictionaries for the auxiliaries exist
  root::DictionaryChecker checker{};
  checker.checkDictionaries<EventAuxiliary>();
  checker.checkDictionaries<SubRunAuxiliary>();
  checker.checkDictionaries<RunAuxiliary>();
  checker.checkDictionaries<ResultsAuxiliary>();
  checker.reportMissingDictionaries();

  createDatabaseTables();
}

void
art::RootOutputFile::createDatabaseTables()
{
  // Event ranges
  create_table(rootFileDB_,
               "EventRanges",
               {"SubRun INTEGER",
                "begin INTEGER",
                "end INTEGER",
                "UNIQUE (SubRun,begin,end) ON CONFLICT IGNORE"});

  // SubRun range sets
  using namespace cet::sqlite;
  create_table(rootFileDB_, "SubRunRangeSets", column<int>{"Run"});
  create_table(rootFileDB_,
               "SubRunRangeSets_EventRanges",
               {"RangeSetsID INTEGER",
                "EventRangesID INTEGER",
                "PRIMARY KEY(RangeSetsID,EventRangesID)"},
               "WITHOUT ROWID");

  // Run range sets
  create_table(rootFileDB_, "RunRangeSets", column<int>{"Run"});
  create_table(rootFileDB_,
               "RunRangeSets_EventRanges",
               {"RangeSetsID INTEGER",
                "EventRangesID INTEGER",
                "PRIMARY KEY(RangeSetsID,EventRangesID)"},
               "WITHOUT ROWID");
}

void
art::RootOutputFile::selectProducts()
{
  for (int i = InEvent; i < NumBranchTypes; ++i) {
    auto const bt = static_cast<BranchType>(i);
    auto& items = selectedOutputItemList_[bt];

    for (auto const& pd : om_->keptProducts()[bt]) {
      // Persist Results products only if they have been produced by
      // the current process.
      if (bt == InResults && !pd->produced())
        continue;
      checkDictionaries(*pd);
      items.emplace(pd);
    }

    for (auto const& val : items) {
      treePointers_[bt]->addOutputBranch(*val.branchDescription_, val.product_);
    }
  }
}

void
art::RootOutputFile::beginInputFile(RootFileBlock const* rfb,
                                    bool const fastCloneFromOutputModule)
{
  // FIXME: the logic here is nasty.
  bool shouldFastClone{fastCloningEnabledAtConstruction_ &&
                       fastCloneFromOutputModule && rfb};
  // Create output branches, and then redo calculation to determine if
  // fast cloning should be done.
  selectProducts();
  if (shouldFastClone &&
      !treePointers_[InEvent]->checkSplitLevelAndBasketSize(rfb->tree())) {
    mf::LogWarning("FastCloning")
      << "Fast cloning deactivated for this input file due to "
      << "splitting level and/or basket size.";
    shouldFastClone = false;
  } else if (rfb && rfb->tree() &&
             rfb->tree()->GetCurrentFile()->GetVersion() < 60001) {
    mf::LogWarning("FastCloning")
      << "Fast cloning deactivated for this input file due to "
      << "ROOT version used to write it (< 6.00/01)\n"
         "having a different splitting policy.";
    shouldFastClone = false;
  }

  if (shouldFastClone && rfb->fileFormatVersion().value_ < 10) {
    mf::LogWarning("FastCloning")
      << "Fast cloning deactivated for this input file due to "
      << "reading in file that has a different ProductID schema.";
    shouldFastClone = false;
  }

  if (shouldFastClone && !fastCloningEnabledAtConstruction_) {
    mf::LogWarning("FastCloning")
      << "Fast cloning reactivated for this input file.";
  }
  treePointers_[InEvent]->beginInputFile(shouldFastClone);
  auto tree = (rfb && rfb->tree()) ? rfb->tree() : nullptr;
  wasFastCloned_ = treePointers_[InEvent]->fastCloneTree(tree);
}

void
art::RootOutputFile::incrementInputFileNumber()
{
  fp_.update<Granularity::InputFile>();
}

void
art::RootOutputFile::respondToCloseInputFile(FileBlock const&)
{
  cet::for_all(treePointers_, [](auto const& p) { p->setEntries(); });
}

bool
art::RootOutputFile::requestsToCloseFile()
{
  using namespace std::chrono;
  unsigned int constexpr oneK{1024u};
  fp_.updateSize(filePtr_->GetSize() / oneK);
  fp_.updateAge(duration_cast<seconds>(steady_clock::now() - beginTime_));
  return fileSwitchCriteria_.should_close(fp_);
}

void
art::RootOutputFile::writeOne(EventPrincipal const& e)
{
  // Auxiliary branch.
  // Note: pEventAux_ must be set before calling fillBranches
  // since it gets written out in that routine.
  pEventAux_ = &e.aux();
  // Because getting the data may cause an exception to be
  // thrown we want to do that first before writing anything
  // to the file about this event.
  fillBranches<InEvent>(e, pEventProductProvenanceVector_);
  // History branch.
  History historyForOutput{e.history()};
  historyForOutput.addEventSelectionEntry(om_->selectorConfig());
  pHistory_ = &historyForOutput;
  int sz = eventHistoryTree_->Fill();
  if (sz <= 0) {
    throw art::Exception(art::errors::FatalRootError)
      << "Failed to fill the History tree for event: " << e.id()
      << "\nTTree::Fill() returned " << sz << " bytes written." << endl;
  }
  // Add the dataType to the job report if it hasn't already been done
  if (!dataTypeReported_) {
    string dataType{"MC"};
    if (pEventAux_->isRealData()) {
      dataType = "Data";
    }
    dataTypeReported_ = true;
  }
  pHistory_ = &e.history();
  // Add event to index
  fileIndex_.addEntry(pEventAux_->id(), fp_.eventEntryNumber());
  fp_.update<Granularity::Event>(status_);
}

void
art::RootOutputFile::writeSubRun(SubRunPrincipal const& sr)
{
  pSubRunAux_ = &sr.aux();
  pSubRunAux_->setRangeSetID(subRunRSID_);
  fillBranches<InSubRun>(sr, pSubRunProductProvenanceVector_);
  fileIndex_.addEntry(EventID::invalidEvent(pSubRunAux_->id()),
                      fp_.subRunEntryNumber());
  fp_.update<Granularity::SubRun>(status_);
}

void
art::RootOutputFile::writeRun(RunPrincipal const& r)
{
  pRunAux_ = &r.aux();
  pRunAux_->setRangeSetID(runRSID_);
  fillBranches<InRun>(r, pRunProductProvenanceVector_);
  fileIndex_.addEntry(EventID::invalidEvent(pRunAux_->id()),
                      fp_.runEntryNumber());
  fp_.update<Granularity::Run>(status_);
}

void
art::RootOutputFile::writeParentageRegistry()
{
  auto pid = root::getObjectRequireDict<ParentageID>();
  ParentageID const* hash = &pid;
  if (!parentageTree_->Branch(
        rootNames::parentageIDBranchName().c_str(), &hash, basketSize_, 0)) {
    throw Exception(errors::FatalRootError)
      << "Failed to create a branch for ParentageIDs in the output file";
  }
  hash = nullptr;

  auto par = root::getObjectRequireDict<Parentage>();
  Parentage const* desc = &par;
  if (!parentageTree_->Branch(
        rootNames::parentageBranchName().c_str(), &desc, basketSize_, 0)) {
    throw art::Exception(art::errors::FatalRootError)
      << "Failed to create a branch for Parentages in the output file";
  }
  desc = nullptr;

  for (auto const& pr : ParentageRegistry::get()) {
    hash = &pr.first;
    desc = &pr.second;
    parentageTree_->Fill();
  }
  parentageTree_->SetBranchAddress(rootNames::parentageIDBranchName().c_str(),
                                   nullptr);
  parentageTree_->SetBranchAddress(rootNames::parentageBranchName().c_str(),
                                   nullptr);
}

void
art::RootOutputFile::writeFileFormatVersion()
{
  FileFormatVersion const ver{getFileFormatVersion(), getFileFormatEra()};
  auto const* pver = &ver;
  TBranch* b = metaDataTree_->Branch(
    metaBranchRootName<FileFormatVersion>(), &pver, basketSize_, 0);
  // FIXME: Turn this into a throw!
  assert(b);
  b->Fill();
}

void
art::RootOutputFile::writeFileIndex()
{
  fileIndex_.sortBy_Run_SubRun_Event();
  FileIndex::Element elem{};
  auto const* findexElemPtr = &elem;
  TBranch* b = fileIndexTree_->Branch(
    metaBranchRootName<FileIndex::Element>(), &findexElemPtr, basketSize_, 0);
  // FIXME: Turn this into a throw!
  assert(b);
  for (auto& entry : fileIndex_) {
    findexElemPtr = &entry;
    b->Fill();
  }
  b->SetAddress(0);
}

void
art::RootOutputFile::writeEventHistory()
{
  RootOutputTree::writeTTree(eventHistoryTree_);
}

void
art::RootOutputFile::writeProcessConfigurationRegistry()
{
  // We don't do this yet; currently we're storing a slightly
  // bloated ProcessHistoryRegistry.
}

void
art::RootOutputFile::writeProcessHistoryRegistry()
{
  ProcessHistoryMap pHistMap;
  for (auto const& pr : ProcessHistoryRegistry::get()) {
    pHistMap.emplace(pr);
  }
  auto const* p = &pHistMap;
  TBranch* b = metaDataTree_->Branch(
    metaBranchRootName<ProcessHistoryMap>(), &p, basketSize_, 0);
  if (b != nullptr) {
    b->Fill();
  } else {
    throw Exception(errors::LogicError) << "Unable to locate required "
                                           "ProcessHistoryMap branch in output "
                                           "metadata tree.\n";
  }
}

void
art::RootOutputFile::writeFileCatalogMetadata(
  FileStatsCollector const& stats,
  FileCatalogMetadata::collection_type const& md,
  FileCatalogMetadata::collection_type const& ssmd)
{
  using namespace cet::sqlite;
  Ntuple<std::string, std::string> fileCatalogMetadata{
    rootFileDB_, "FileCatalog_metadata", {"Name", "Value"}, true};
  Transaction txn{rootFileDB_};
  for (auto const& kv : md) {
    fileCatalogMetadata.insert(kv.first, kv.second);
  }
  // Add our own specific information: File format and friends.
  fileCatalogMetadata.insert("file_format", "\"artroot\"");
  fileCatalogMetadata.insert("file_format_era",
                             cet::canonical_string(getFileFormatEra()));
  fileCatalogMetadata.insert("file_format_version",
                             to_string(getFileFormatVersion()));

  // File start time.
  namespace bpt = boost::posix_time;
  auto formatted_time = [](auto const& t) {
    return cet::canonical_string(bpt::to_iso_extended_string(t));
  };
  fileCatalogMetadata.insert("start_time",
                             formatted_time(stats.outputFileOpenTime()));
  // File "end" time: now, since file is not actually closed yet.
  fileCatalogMetadata.insert(
    "end_time",
    formatted_time(boost::posix_time::second_clock::universal_time()));

  // Run/subRun information.
  if (!stats.seenSubRuns().empty()) {

    auto I = find_if(md.crbegin(), md.crend(), [](auto const& p) {
      return p.first == "run_type";
    });

    if (I != md.crend()) {
      ostringstream buf;
      buf << "[ ";
      for (auto const& srid : stats.seenSubRuns()) {
        buf << "[ " << srid.run() << ", " << srid.subRun() << ", "
            << cet::canonical_string(I->second) << " ], ";
      }
      // Rewind over last delimiter.
      buf.seekp(-2, ios_base::cur);
      buf << " ]";
      fileCatalogMetadata.insert("runs", buf.str());
    }
  }
  // Number of events.
  fileCatalogMetadata.insert("event_count", to_string(stats.eventsThisFile()));
  // first_event and last_event.
  auto eidToTuple = [](EventID const& eid) -> string {
    ostringstream eidStr;
    eidStr << "[ " << eid.run() << ", " << eid.subRun() << ", " << eid.event()
           << " ]";
    return eidStr.str();
  };
  fileCatalogMetadata.insert("first_event", eidToTuple(stats.lowestEventID()));
  fileCatalogMetadata.insert("last_event", eidToTuple(stats.highestEventID()));
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
    fileCatalogMetadata.insert("parents", pstring.str());
  }
  // Incoming stream-specific metadata overrides.
  for (auto const& kv : ssmd) {
    fileCatalogMetadata.insert(kv.first, kv.second);
  }
  txn.commit();
}

void
art::RootOutputFile::writeParameterSetRegistry()
{
  fhicl::ParameterSetRegistry::exportTo(rootFileDB_);
}

void
art::RootOutputFile::writeProductDescriptionRegistry()
{
  // Make a local copy of the MasterProductRegistry's ProductList,
  // removing any transient or pruned products.
  auto end = branchesWithStoredHistory_.end();

  ProductRegistry reg;
  for (auto const& pr : ProductMetaData::instance().productList()) {
    if (branchesWithStoredHistory_.find(pr.second.productID()) == end) {
      continue;
    }
    reg.productList_.emplace_hint(reg.productList_.end(), pr);
  }

  ProductRegistry const* regp = &reg;
  TBranch* b = metaDataTree_->Branch(
    metaBranchRootName<ProductRegistry>(), &regp, basketSize_, 0);
  // FIXME: Turn this into a throw!
  assert(b);
  b->Fill();
}

void
art::RootOutputFile::writeProductDependencies()
{
  BranchChildren const* ppDeps = &om_->branchChildren();
  TBranch* b = metaDataTree_->Branch(
    metaBranchRootName<BranchChildren>(), &ppDeps, basketSize_, 0);
  // FIXME: Turn this into a throw!
  assert(b);
  b->Fill();
}

void
art::RootOutputFile::writeResults(ResultsPrincipal& resp)
{
  pResultsAux_ = &resp.aux();
  fillBranches<InResults>(resp, pResultsProductProvenanceVector_);
}

void
art::RootOutputFile::writeTTrees()
{
  RootOutputTree::writeTTree(metaDataTree_);
  RootOutputTree::writeTTree(fileIndexTree_);
  RootOutputTree::writeTTree(parentageTree_);
  // Write out the tree corresponding to each BranchType
  for (int i = InEvent; i < NumBranchTypes; ++i) {
    auto const branchType = static_cast<BranchType>(i);
    treePointers_[branchType]->writeTree();
  }
}

template <art::BranchType BT>
void
art::RootOutputFile::fillBranches(Principal const& principal,
                                  vector<ProductProvenance>* vpp)
{
  bool const fastCloning = (BT == InEvent) && wasFastCloned_;
  detail::KeptProvenance keptProvenance{
    dropMetaData_, dropMetaDataForDroppedData_, branchesWithStoredHistory_};
  map<unsigned, unsigned> checksumToIndex;

  auto const& principalRS = principal.seenRanges();

  for (auto const& val : selectedOutputItemList_[BT]) {
    auto const* pd = val.branchDescription_;
    auto const pid = pd->productID();
    branchesWithStoredHistory_.insert(pid);
    bool const produced{pd->produced()};
    bool const resolveProd = (produced || !fastCloning ||
                              treePointers_[BT]->uncloned(pd->branchName()));

    // Update the kept provenance
    bool const keepProvenance =
      (dropMetaData_ == DropMetaData::DropNone ||
       (dropMetaData_ == DropMetaData::DropPrior && produced));
    auto const& oh = principal.getForOutput(pid, resolveProd);

    unique_ptr<ProductProvenance> prov{nullptr};
    if (keepProvenance) {
      if (oh.productProvenance()) {
        prov = std::make_unique<ProductProvenance>(
          keptProvenance.insert(*oh.productProvenance()));
        keptProvenance.insertAncestors(*oh.productProvenance(), principal);
      } else {
        // No provenance, product was either not produced, or was
        // dropped, create provenance to remember that.
        auto const status =
          produced ? productstatus::neverCreated() : productstatus::dropped();
        prov = std::make_unique<ProductProvenance>(
          keptProvenance.emplace(pid, status));
      }
    }

    // Resolve the product if necessary
    if (resolveProd) {
      auto const& rs = getRangeSet<BT>(oh, principalRS, produced);
      if (RangeSetsSupported<BT>::value && !rs.is_valid()) {
        // Unfortunately, 'unknown' is the only viable product status
        // when this condition is triggered (due to the assert
        // statement in ProductStatus::setNotPresent).  Whenever the
        // metadata revolution comes, this should be revised.
        keptProvenance.setStatus(*prov, productstatus::unknown());
      }

      auto const* product = getProduct<BT>(oh, rs, pd->wrappedName());
      setProductRangeSetID<BT>(
        rs, rootFileDB_, const_cast<EDProduct*>(product), checksumToIndex);
      val.product_ = product;
    }
  }
  vpp->assign(keptProvenance.begin(), keptProvenance.end());
  treePointers_[BT]->fillTree();
  vpp->clear();
}

void
art::RootOutputFile::setSubRunAuxiliaryRangeSetID(RangeSet const& ranges)
{
  subRunRSID_ = getNewRangeSetID(rootFileDB_, InSubRun, ranges.run());
  insertIntoEventRanges(rootFileDB_, ranges);
  auto const& eventRangesIDs = getExistingRangeSetIDs(rootFileDB_, ranges);
  insertIntoJoinTable(rootFileDB_, InSubRun, subRunRSID_, eventRangesIDs);
}

void
art::RootOutputFile::setRunAuxiliaryRangeSetID(RangeSet const& ranges)
{
  runRSID_ = getNewRangeSetID(rootFileDB_, InRun, ranges.run());
  insertIntoEventRanges(rootFileDB_, ranges);
  auto const& eventRangesIDs = getExistingRangeSetIDs(rootFileDB_, ranges);
  insertIntoJoinTable(rootFileDB_, InRun, runRSID_, eventRangesIDs);
}

template <BranchType BT>
std::enable_if_t<!RangeSetsSupported<BT>::value, art::EDProduct const*>
art::RootOutputFile::getProduct(art::OutputHandle const& oh,
                                art::RangeSet const& /*prunedProductRS*/,
                                std::string const& wrappedName)
{
  return oh.isValid() ? oh.wrapper() : dummyProductCache_.product(wrappedName);
}

template <BranchType BT>
std::enable_if_t<RangeSetsSupported<BT>::value, art::EDProduct const*>
art::RootOutputFile::getProduct(art::OutputHandle const& oh,
                                art::RangeSet const& prunedProductRS,
                                std::string const& wrappedName)
{
  return (oh.isValid() && prunedProductRS.is_valid()) ?
           oh.wrapper() :
           dummyProductCache_.product(wrappedName);
}
