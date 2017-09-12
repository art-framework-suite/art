#include "art/Framework/IO/Root/RootInputFile.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/GroupSelector.h"
#include "art/Framework/IO/Root/DuplicateChecker.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/IO/Root/RootDelayedReader.h"
#include "art/Framework/IO/Root/checkDictionaries.h"
#include "art/Framework/IO/Root/detail/readFileIndex.h"
#include "art/Framework/IO/Root/detail/readMetadata.h"
#include "art/Framework/IO/Root/detail/resolveRangeSet.h"
#include "art/Framework/Principal/ClosedRangeSetHandler.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/System/DatabaseConnection.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/RootDB/TKeyVFSOpenPolicy.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/BranchChildren.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ParameterSetBlob.h"
#include "canvas/Persistency/Provenance/ParameterSetMap.h"
#include "canvas/Persistency/Provenance/ParentageRegistry.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductIDStreamer.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/FriendlyName.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "TBranch.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>

extern "C" {
#include "sqlite3.h"
}

using namespace cet;
using namespace std;


namespace {

  bool have_table(sqlite3* db,
                  string const& table,
                  string const& filename)
  {
    bool result = false;
    sqlite3_stmt* stmt = nullptr;
    string const ddl {"select 1 from sqlite_master where type='table' and name='" + table + "';"};
    auto rc = sqlite3_prepare_v2(db, ddl.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
      switch (rc = sqlite3_step(stmt)) {
      case SQLITE_ROW:
        result = true; // Found the table.
      case SQLITE_DONE:
        rc = SQLITE_OK; // No such table.
        break;
      default:
        break;
      }
    }
    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
      throw art::Exception(art::errors::FileReadError)
        << "Error interrogating SQLite3 DB in file " << filename
        << ".\n";
    }
    return result;
  }

} // unnamed namespace


namespace art {

  using std::swap;

  namespace detail {

    void
    mergeAuxiliary(RunAuxiliary& left, RunAuxiliary const& right)
    {
      left.mergeAuxiliary(right);
    }

    void
    mergeAuxiliary(SubRunAuxiliary& left, SubRunAuxiliary const& right)
    {
      left.mergeAuxiliary(right);
    }

  } // namespace detail


  RootInputFile::RootInputTree::
  ~RootInputTree()
  {
  }

  RootInputFile::RootInputTree::
  RootInputTree(cet::exempt_ptr<TFile> filePtr, BranchType const branchType, int64_t /*saveMemoryObjectThreshold*/,
                cet::exempt_ptr<RootInputFile> /*primaryFile*/, bool const missingOK /*=false*/)
  {
    if (filePtr) {
      tree_ = static_cast<TTree*>(filePtr->Get(BranchTypeToProductTreeName(branchType).c_str()));
      metaTree_ = static_cast<TTree*>(filePtr->Get(BranchTypeToMetaDataTreeName(branchType).c_str()));
    }
    if (tree_) {
      auxBranch_ = tree_->GetBranch(BranchTypeToAuxiliaryBranchName(branchType).c_str());
      entries_ = tree_->GetEntries();
    }
    if (metaTree_) {
      productProvenanceBranch_ = metaTree_->GetBranch(productProvenanceBranchName(branchType).c_str());
    }
    if (!missingOK && !isValid()) {
      throw Exception(errors::FileReadError)
        << "RootInputTree for branch type "
        << BranchTypeToString(branchType)
        << " could not be initialized correctly from input file.\n";
    }
  }

  TBranch*
  RootInputFile::RootInputTree::
  auxBranch() const
  {
    return auxBranch_;
  }

  RootInputFile::RootInputTree::EntryNumber
  RootInputFile::RootInputTree::
  entries() const
  {
    return entries_;
  }

  TTree*
  RootInputFile::RootInputTree::
  tree() const
  {
    return tree_;
  }

  TTree*
  RootInputFile::RootInputTree::
  metaTree() const
  {
    return metaTree_;
  }

  RootInputFile::RootInputTree::BranchMap const&
  RootInputFile::RootInputTree::
  branches() const
  {
    return branches_;
  }

  TBranch*
  RootInputFile::RootInputTree::
  productProvenanceBranch() const
  {
    return productProvenanceBranch_;
  }

  bool
  RootInputFile::RootInputTree::
  isValid() const
  {
    if ((metaTree_ == nullptr) || (metaTree_->GetNbranches() == 0)) {
      return tree_ && auxBranch_ && (tree_->GetNbranches() == 1);
    }
    return tree_ && auxBranch_ && metaTree_ && productProvenanceBranch_;
  }

  void
  RootInputFile::RootInputTree::
  addBranch(BranchKey const& key, BranchDescription const& bd)
  {
    assert(isValid());
    TBranch* branch = tree_->GetBranch(bd.branchName().c_str());
    assert(bd.present() == (branch != nullptr));
    assert(bd.dropped() == (branch == nullptr));
    input::BranchInfo info{bd, branch};
    branches_.emplace(key, std::move(info));
  }

  void
  RootInputFile::RootInputTree::
  dropBranch(string const& branchName)
  {
    TBranch* branch = tree_->GetBranch(branchName.c_str());
    if (branch == nullptr) {
      return;
    }
    TObjArray* leaves = tree_->GetListOfLeaves();
    if (leaves == nullptr) {
      return;
    }
    int entries = leaves->GetEntries();
    for (int i = 0; i < entries; ++i) {
      TLeaf* leaf = reinterpret_cast<TLeaf*>((*leaves)[i]);
      if (leaf == nullptr) {
        continue;
      }
      TBranch* br = leaf->GetBranch();
      if (br == nullptr) {
        continue;
      }
      if (br->GetMother() == branch) {
        leaves->Remove(leaf);
      }
    }
    leaves->Compress();
    tree_->GetListOfBranches()->Remove(branch);
    tree_->GetListOfBranches()->Compress();
    delete branch;
    branch = nullptr;
  }

  RootInputFile::
  ~RootInputFile()
  {
  }

  RootInputFile::
  RootInputFile(string const& fileName,
                string const& catalogName,
                ProcessConfiguration const& processConfiguration,
                string const& logicalFileName,
                unique_ptr<TFile>&& filePtr,
                EventID const& origEventID,
                unsigned int eventsToSkip,
                FastCloningInfoProvider const& fcip,
                unsigned int treeCacheSize,
                int64_t treeMaxVirtualSize,
                int64_t saveMemoryObjectThreshold,
                bool delayedReadEventProducts,
                bool delayedReadSubRunProducts,
                bool delayedReadRunProducts,
                InputSource::ProcessingMode processingMode,
                int forcedRunOffset,
                bool noEventSort,
                GroupSelectorRules const& groupSelectorRules,
                shared_ptr<DuplicateChecker> duplicateChecker,
                bool dropDescendants,
                bool const readIncomingParameterSets,
                exempt_ptr<RootInputFile> primaryFile,
                vector<string> const& secondaryFileNames,
                RootInputFileSequence* rifSequence,
                MasterProductRegistry& mpr)
  : fileName_{fileName}
  , catalog_{catalogName}
  , processConfiguration_{processConfiguration}
  , logicalFileName_{logicalFileName}
  , filePtr_{move(filePtr)}
  , origEventID_{origEventID}
  , eventsToSkip_{eventsToSkip}
  , treePointers_ { // Indexed by BranchTypes.h!
      make_unique<RootInputTree>(filePtr_.get(), InEvent, saveMemoryObjectThreshold, this, false),
      make_unique<RootInputTree>(filePtr_.get(), InSubRun, saveMemoryObjectThreshold, this, false),
      make_unique<RootInputTree>(filePtr_.get(), InRun, saveMemoryObjectThreshold, this, false),
      make_unique<RootInputTree>(filePtr_.get(), InResults, saveMemoryObjectThreshold, this, true)
    }
  , delayedReadEventProducts_{delayedReadEventProducts}
  , delayedReadSubRunProducts_{delayedReadSubRunProducts}
  , delayedReadRunProducts_{delayedReadRunProducts}
  , processingMode_{processingMode}
  , forcedRunOffset_{forcedRunOffset}
  , noEventSort_{noEventSort}
  , duplicateChecker_{duplicateChecker}
  , primaryFile_{primaryFile ? primaryFile : this}
  , secondaryFileNames_{secondaryFileNames}
  , rifSequence_{rifSequence}
  , saveMemoryObjectThreshold_{saveMemoryObjectThreshold}
  {
    secondaryFiles_.resize(secondaryFileNames_.size());
    if (treeMaxVirtualSize >= 0) {
      eventTree().tree()->SetMaxVirtualSize(static_cast<Long64_t>(treeMaxVirtualSize));
    }
    if (treeMaxVirtualSize >= 0) {
      subRunTree().tree()->SetMaxVirtualSize(static_cast<Long64_t>(treeMaxVirtualSize));
    }
    if (treeMaxVirtualSize >= 0) {
      runTree().tree()->SetMaxVirtualSize(static_cast<Long64_t>(treeMaxVirtualSize));
    }
    eventTree().tree()->SetCacheSize(static_cast<Long64_t>(treeCacheSize));
    subRunTree().tree()->SetCacheSize(static_cast<Long64_t>(treeCacheSize));
    runTree().tree()->SetCacheSize(static_cast<Long64_t>(treeCacheSize));
    if (resultsTree().isValid()) {
      if (treeMaxVirtualSize >= 0) {
        resultsTree().tree()->SetMaxVirtualSize(static_cast<Long64_t>(treeMaxVirtualSize));
      }
      resultsTree().tree()->SetCacheSize(static_cast<Long64_t>(treeCacheSize));
    }
    // Retrieve the metadata tree.
    auto metaDataTree = static_cast<TTree*>(filePtr_->Get(rootNames::metaDataTreeName().c_str()));
    if (!metaDataTree) {
      throw art::Exception{errors::FileReadError}
      << couldNotFindTree(rootNames::metaDataTreeName());
    }
    metaDataTree->SetCacheSize(static_cast<Long64_t>(treeCacheSize));
    using namespace art::rootNames;
    fileFormatVersion_ = detail::readMetadata<FileFormatVersion>(metaDataTree);
    // Read file index
    auto findexPtr = &fileIndex_;
    detail::readFileIndex(filePtr_.get(), metaDataTree, findexPtr);
    // To support files that contain BranchIDLists
    BranchIDLists branchIDLists;
    if (detail::readMetadata(metaDataTree, branchIDLists)) {
      branchIDLists_ = make_unique<BranchIDLists>(move(branchIDLists));
      configureProductIDStreamer(branchIDLists_.get());
    }
    // Read the ParameterSets if there are any on a branch.
    {
      ParameterSetMap psetMap;
      if (readIncomingParameterSets && detail::readMetadata(metaDataTree, psetMap)) {
        // Merge into the hashed registries.
        for (auto const& psEntry : psetMap) {
          fhicl::ParameterSet pset;
          fhicl::make_ParameterSet(psEntry.second.pset_, pset);
          // Note ParameterSet::id() has the side effect of making
          // sure the parameter set *has* an ID.
          pset.id();
          fhicl::ParameterSetRegistry::put(pset);
        }
      }
    }
    // Read the ProcessHistory
    {
      auto pHistMap = detail::readMetadata<ProcessHistoryMap>(metaDataTree);
      ProcessHistoryRegistry::put(pHistMap);
    }
    // Check the, "Era" of the input file (new since art v0.5.0). If it
    // does not match what we expect we cannot read the file. Required
    // since we reset the file versioning since forking off from
    // CMS. Files written by art prior to v0.5.0 will *also* not be
    // readable because they do not have this datum and because the run,
    // subrun and event-number handling has changed significantly.
    string const& expected_era = art::getFileFormatEra();
    if (fileFormatVersion_.era_ != expected_era) {
      throw art::Exception{art::errors::FileReadError}
      << "Can only read files written during the \""
           << expected_era
           << "\" era: "
           << "Era of "
           << "\""
           << fileName_
           << "\" was "
           << (fileFormatVersion_.era_.empty() ?
               "not set" :
               ("set to \"" + fileFormatVersion_.era_ + "\" "))
           << ".\n";
    }
    // Also need to check RootFileDB if we have one.
    if (fileFormatVersion_.value_ >= 5) {
      sqliteDB_ = ServiceHandle<DatabaseConnection> {}->get<TKeyVFSOpenPolicy>("RootFileDB", filePtr_.get());
      if (readIncomingParameterSets && have_table(sqliteDB_, "ParameterSets", fileName_)) {
        fhicl::ParameterSetRegistry::importFrom(sqliteDB_);
      }
      if (ServiceRegistry::instance().isAvailable<art::FileCatalogMetadata>() &&
          have_table(sqliteDB_, "FileCatalog_metadata", fileName_)) {
        sqlite3_stmt* stmt {nullptr};
        sqlite3_prepare_v2(sqliteDB_,
                           "SELECT Name, Value from FileCatalog_metadata;",
                           -1,
                           &stmt,
                           nullptr);
        vector<pair<string, string>> md;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
          string const name {reinterpret_cast<char const*>(sqlite3_column_text(stmt, 0))};
          string const value {reinterpret_cast<char const*>(sqlite3_column_text(stmt, 1))};
          md.emplace_back(name, value);
        }
        int const finalize_status = sqlite3_finalize(stmt);
        if (finalize_status != SQLITE_OK) {
          throw art::Exception{art::errors::SQLExecutionError}
          << "Unexpected status from DB status cleanup: "
               << sqlite3_errmsg(sqliteDB_)
               << " (0x"
               << finalize_status
               << ").\n";
        }
        art::ServiceHandle<art::FileCatalogMetadata> {}->setMetadataFromInput(md);
      }
    }
    validateFile();
    // Read the parentage tree.  Old format files are handled
    // internally in readParentageTree().
    readParentageTree(treeCacheSize);
    initializeDuplicateChecker();
    if (noEventSort_) {
      fileIndex_.sortBy_Run_SubRun_EventEntry();
    }
    fiIter_ = fileIndex_.begin();
    fiBegin_ = fileIndex_.begin();
    fiEnd_ = fileIndex_.end();
    readEventHistoryTree(treeCacheSize);

    // Read the ProductList
    productListHolder_ = detail::readMetadata<ProductRegistry>(metaDataTree);
    auto& prodList = productListHolder_.productList_;
    dropOnInput(groupSelectorRules, dropDescendants, prodList);

    auto availableProducts = fillPerBranchTypePresenceFlags(prodList);

    // Add branches
    for (auto& prod : prodList) {
      auto& pd = prod.second;
      auto const& presenceLookup = availableProducts[pd.branchType()];
      bool const present{presenceLookup.find(pd.productID()) != cend(presenceLookup)};
      auto const validity = present ? BranchDescription::Transients::PresentFromSource : BranchDescription::Transients::Dropped;
      pd.setValidity(validity);
      treePointers_[pd.branchType()]->addBranch(prod.first, pd);
    }

    // Update MasterProductRegistry, with adjusted BranchDescription
    // validity values.
    mpr.updateFromInputFile(prodList);

    auto const& descriptions = make_product_descriptions(prodList);
    presentProducts_ = ProductTables{descriptions, availableProducts};

    // Determine if this file is fast clonable.
    fastClonable_ = setIfFastClonable(fcip);
    reportOpened();
    // FIXME: This probably is unnecessary!
    configureProductIDStreamer();
  }

  bool
  RootInputFile::
  setEntry_Event(EventID const& id, bool exact /*= true*/)
  {
    fiIter_ = fileIndex_.findPosition(id, exact);
    if (fiIter_ == fiEnd_) {
      return false;
    }
    return true;
  }

  bool
  RootInputFile::
  setEntry_SubRun(SubRunID const& id, bool exact /*= true*/)
  {
    fiIter_ = fileIndex_.findPosition(id, exact);
    if (fiIter_ == fiEnd_) {
      return false;
    }
    return true;
  }

  bool
  RootInputFile::
  setEntry_Run(RunID const& id, bool exact /*= true*/)
  {
    fiIter_ = fileIndex_.findPosition(id, exact);
    if (fiIter_ == fiEnd_) {
      return false;
    }
    return true;
  }

  RootInputFile::RootInputTree const&
  RootInputFile::
  eventTree() const
  {
    return *treePointers_[InEvent];
  }

  RootInputFile::RootInputTree const&
  RootInputFile::
  subRunTree() const
  {
    return *treePointers_[InSubRun];
  }

  RootInputFile::RootInputTree const&
  RootInputFile::
  runTree() const
  {
    return *treePointers_[InRun];
  }

  RootInputFile::RootInputTree const&
  RootInputFile::
  resultsTree() const
  {
    return *treePointers_[InResults];
  }

  RootInputFile::RootInputTree&
  RootInputFile::
  eventTree()
  {
    return *treePointers_[InEvent];
  }

  RootInputFile::RootInputTree&
  RootInputFile::
  subRunTree()
  {
    return *treePointers_[InSubRun];
  }

  RootInputFile::RootInputTree&
  RootInputFile::
  runTree()
  {
    return *treePointers_[InRun];
  }

  RootInputFile::RootInputTree&
  RootInputFile::
  resultsTree()
  {
    return *treePointers_[InResults];
  }

  void
  RootInputFile::
  fillAuxiliary_Event(EntryNumber const entry)
  {
    //using AUX = tuple_element_t<InEvent, decltype(auxiliaries_)>;
    //auto& aux = get<InEvent>(auxiliaries_);
    //aux = treePointers_[InEvent]->getAux_Event(entry);
    //eventAux_ = treePointers_[InEvent]->getAux_Event(entry);
    auto auxbr = treePointers_[InEvent]->auxBranch();
    auto pAux = &eventAux_;
    auxbr->SetAddress(&pAux);
    //setEntryNumber(entry);
    input::getEntry(auxbr, entry);
  }

  void
  RootInputFile::
  fillAuxiliary_SubRun(EntryNumber const entry)
  {
    //using AUX = tuple_element_t<InSubRun, decltype(auxiliaries_)>;
    //auto& aux = get<InSubRun>(auxiliaries_);
    //aux = treePointers_[InSubRun]->getAux_SubRun(entry);
    //subRunAux_ = treePointers_[InSubRun]->getAux_SubRun(entry);
    auto auxbr = treePointers_[InSubRun]->auxBranch();
    auto pAux = &subRunAux_;
    auxbr->SetAddress(&pAux);
    //setEntryNumber(entry);
    input::getEntry(auxbr, entry);
  }

  void
  RootInputFile::
  fillAuxiliary_Run(EntryNumber const entry)
  {
    //using AUX = tuple_element_t<InRun, decltype(auxiliaries_)>;
    //auto& aux = get<InRun>(auxiliaries_);
    //aux = treePointers_[InRun]->getAux_Run(entry);
    //runAux_ = treePointers_[InRun]->getAux_Run(entry);
    auto auxbr = treePointers_[InRun]->auxBranch();
    auto pAux = &runAux_;
    auxbr->SetAddress(&pAux);
    //setEntryNumber(entry);
    input::getEntry(auxbr, entry);
  }

  void
  RootInputFile::
  fillAuxiliary_Results(EntryNumber const entry)
  {
    //using AUX = tuple_element_t<InResults, decltype(auxiliaries_)>;
    //auto& aux = get<InResults>(auxiliaries_);
    //aux = treePointers_[InResults]->getAux_Results(entry);
    //resultsAux_ = treePointers_[InResults]->getAux_Results(entry);
    auto auxbr = treePointers_[InResults]->auxBranch();
    auto pAux = &resultsAux_;
    auxbr->SetAddress(&pAux);
    //setEntryNumber(entry);
    input::getEntry(auxbr, entry);
  }

  unique_ptr<RangeSetHandler>
  RootInputFile::
  fillAuxiliary_SubRun(EntryNumbers const& entries)
  {
    //using AUX = tuple_element_t<InSubRun, decltype(auxiliaries_)>;
    //auto& aux = get<InSubRun>(auxiliaries_);
    //return treePointers_[InSubRun]->fillAux_SubRun(fileFormatVersion_, entries, sqliteDB_, fileName_, aux);
    //return treePointers_[InSubRun]->fillAux_SubRun(fileFormatVersion_, entries, sqliteDB_, fileName_, subRunAux_);
    //auto auxResult = getAux_SubRun(entries[0]);
    SubRunAuxiliary auxResult{};
    {
      auto auxbr = treePointers_[InSubRun]->auxBranch();
      auto pAux = &auxResult;
      auxbr->SetAddress(&pAux);
      //setEntryNumber(entry);
      input::getEntry(auxbr, entries[0]);
    }
    if (fileFormatVersion_.value_ < 9) {
      swap(subRunAux_, auxResult);
      return make_unique<OpenRangeSetHandler>(subRunAux_.run());
    }
    auto resolve_info = [this](auto const id) {
      return detail::resolveRangeSetInfo(this->sqliteDB_, this->fileName_, SubRunAuxiliary::branch_type, id);
    };
    auto rangeSetInfo = resolve_info(auxResult.rangeSetID());
    for (auto i = entries.cbegin() + 1, e = entries.cend(); i != e; ++i) {
      //auto const& tmpAux = getAux_SubRun(*i);
      SubRunAuxiliary tmpAux{};
      {
        auto auxbr = treePointers_[InSubRun]->auxBranch();
        auto pAux = &tmpAux;
        auxbr->SetAddress(&pAux);
        //setEntryNumber(entry);
        input::getEntry(auxbr, *i);
      }
      detail::mergeAuxiliary(auxResult, tmpAux);
      rangeSetInfo.update(resolve_info(tmpAux.rangeSetID()));
    }
    auxResult.setRangeSetID(-1u); // Range set of new auxiliary is invalid
    swap(subRunAux_, auxResult);
    return make_unique<ClosedRangeSetHandler>(resolveRangeSet(rangeSetInfo));
  }

  unique_ptr<RangeSetHandler>
  RootInputFile::
  fillAuxiliary_Run(EntryNumbers const& entries)
  {
    //using AUX = tuple_element_t<InRun, decltype(auxiliaries_)>;
    //auto& aux = get<InRun>(auxiliaries_);
    //return treePointers_[InRun]->fillAux_Run(fileFormatVersion_, entries, sqliteDB_, fileName_, aux);
    //auto auxResult = getAux_Run(entries[0]);
    RunAuxiliary auxResult{};
    {
      auto auxbr = treePointers_[InRun]->auxBranch();
      auto pAux = &auxResult;
      auxbr->SetAddress(&pAux);
      //setEntryNumber(entry);
      input::getEntry(auxbr, entries[0]);
    }
    if (fileFormatVersion_.value_ < 9) {
      swap(runAux_, auxResult);
      return make_unique<OpenRangeSetHandler>(runAux_.run());
    }
    auto resolve_info = [this](auto const id) {
      return detail::resolveRangeSetInfo(this->sqliteDB_, this->fileName_, RunAuxiliary::branch_type, id);
    };
    auto rangeSetInfo = resolve_info(auxResult.rangeSetID());
    for (auto i = entries.cbegin() + 1, e = entries.cend(); i != e; ++i) {
      //auto const& tmpAux = getAux_Run(*i);
      RunAuxiliary tmpAux{};
      {
        auto auxbr = treePointers_[InRun]->auxBranch();
        auto pAux = &tmpAux;
        auxbr->SetAddress(&pAux);
        //setEntryNumber(entry);
        input::getEntry(auxbr, *i);
      }
      detail::mergeAuxiliary(auxResult, tmpAux);
      rangeSetInfo.update(resolve_info(tmpAux.rangeSetID()));
    }
    auxResult.setRangeSetID(-1u); // Range set of new auxiliary is invalid
    swap(runAux_, auxResult);
    return make_unique<ClosedRangeSetHandler>(resolveRangeSet(rangeSetInfo));
  }

  string const&
  RootInputFile::
  fileName() const
  {
    return fileName_;
  }


  RootInputFile::RootInputTreePtrArray&
  RootInputFile::
  treePointers()
  {
    return treePointers_;
  }

  FileFormatVersion
  RootInputFile::
  fileFormatVersion() const
  {
    return fileFormatVersion_;
  }

  bool
  RootInputFile::
  fastClonable() const
  {
    return fastClonable_;
  }

  void
  RootInputFile::
  rewind()
  {
    fiIter_ = fiBegin_;
    // FIXME: Rewinding the trees is suspicious!
    // FIXME: They should be positioned based on the new iter pos.
    //eventTree().rewind();
    //subRunTree().rewind();
    //runTree().rewind();
  }

  void
  RootInputFile::
  setToLastEntry()
  {
    fiIter_ = fiEnd_;
  }

  void
  RootInputFile::
  nextEntry()
  {
    ++fiIter_;
  }

  void
  RootInputFile::
  previousEntry()
  {
    --fiIter_;
  }

  void
  RootInputFile::
  advanceEntry(size_t n)
  {
    while (n-- != 0) {
      nextEntry();
    }
  }

  unsigned int
  RootInputFile::
  eventsToSkip() const
  {
    return eventsToSkip_;
  }

  shared_ptr<FileIndex>
  RootInputFile::
  fileIndexSharedPtr() const
  {
    return fileIndexSharedPtr_;
  }

  vector<string> const&
  RootInputFile::
  secondaryFileNames() const
  {
    return secondaryFileNames_;
  }

  vector<unique_ptr<RootInputFile>> const&
  RootInputFile::
  secondaryFiles() const
  {
    return secondaryFiles_;
  }

  void
  RootInputFile::
  readParentageTree(unsigned int const treeCacheSize)
  {
    //
    //  Auxiliary routine for the constructor.
    //
    auto parentageTree = static_cast<TTree*>(filePtr_->Get(rootNames::parentageTreeName().c_str()));
    if (!parentageTree) {
      throw art::Exception{errors::FileReadError}
      << couldNotFindTree(rootNames::parentageTreeName());
    }
    parentageTree->SetCacheSize(static_cast<Long64_t>(treeCacheSize));
    ParentageID idBuffer;
    auto pidBuffer = &idBuffer;
    parentageTree->SetBranchAddress(rootNames::parentageIDBranchName().c_str(), &pidBuffer);
    Parentage parentageBuffer;
    auto pParentageBuffer = &parentageBuffer;
    parentageTree->SetBranchAddress(rootNames::parentageBranchName().c_str(), &pParentageBuffer);
    // Fill the registry
    for (EntryNumber i = 0, numEntries = parentageTree->GetEntries(); i < numEntries; ++i) {
      input::getEntry(parentageTree, i);
      if (idBuffer != parentageBuffer.id()) {
        throw art::Exception{errors::DataCorruption}
        << "Corruption of Parentage tree detected.\n";
      }
      ParentageRegistry::emplace(parentageBuffer.id(), parentageBuffer);
    }
    parentageTree->SetBranchAddress(rootNames::parentageIDBranchName().c_str(), nullptr);
    parentageTree->SetBranchAddress(rootNames::parentageBranchName().c_str(), nullptr);
  }

  EventID
  RootInputFile::
  eventIDForFileIndexPosition() const
  {
    if (fiIter_ == fiEnd_) {
      return EventID{};
    }
    return fiIter_->eventID_;
  }

  bool
  RootInputFile::
  setIfFastClonable(FastCloningInfoProvider const& fcip) const
  {
    if (!fcip.fastCloningPermitted()) {
      return false;
    }
    if (!fileFormatVersion_.fastCopyPossible()) {
      return false;
    }
    if (secondaryFileNames_.size() != 0) {
      return false;
    }
    if (!fileIndex_.allEventsInEntryOrder()) {
      return false;
    }
    if (eventsToSkip_ != 0) {
      return false;
    }
    if ((fcip.remainingEvents() >= 0) &&
        (eventTree().entries() > fcip.remainingEvents())) {
      return false;
    }
    if ((fcip.remainingSubRuns() >= 0) &&
        (subRunTree().entries() > fcip.remainingSubRuns())) {
      return false;
    }
    if (processingMode_ != InputSource::RunsSubRunsAndEvents) {
      return false;
    }
    if (forcedRunOffset_ != 0) {
      return false;
    }
    // Find entry for first event in file.
    auto it = fiBegin_;
    while ((it != fiEnd_) && (it->getEntryType() != FileIndex::kEvent)) {
      ++it;
    }
    if (it == fiEnd_) {
      return false;
    }
    if (it->eventID_ < origEventID_) {
      return false;
    }
    return true;
  }

  int
  RootInputFile::
  setForcedRunOffset(RunNumber_t const& forcedRunNumber)
  {
    if (fiBegin_ == fiEnd_) {
      return 0;
    }
    forcedRunOffset_ = 0;
    if (!RunID(forcedRunNumber).isValid()) {
      return 0;
    }
    forcedRunOffset_ = forcedRunNumber - fiBegin_->eventID_.run();
    if (forcedRunOffset_ != 0) {
      fastClonable_ = false;
    }
    return forcedRunOffset_;
  }

  unique_ptr<FileBlock>
  RootInputFile::
  createFileBlock()
  {
    return std::make_unique<FileBlock>(fileFormatVersion_,
                                       fileName_,
                                       readResults(),
                                       cet::make_exempt_ptr(eventTree().tree()),
                                       fastClonable());
  }

  FileIndex::EntryType
  RootInputFile::
  getEntryType() const
  {
    if (fiIter_ == fiEnd_) {
      return FileIndex::kEnd;
    }
    return fiIter_->getEntryType();
  }

  FileIndex::EntryType
  RootInputFile::
  getNextEntryTypeWanted()
  {
    auto entryType = getEntryType();
    if (entryType == FileIndex::kEnd) {
      return FileIndex::kEnd;
    }
    RunID currentRun(fiIter_->eventID_.runID());
    if (!currentRun.isValid()) {
      return FileIndex::kEnd;
    }
    if (entryType == FileIndex::kRun) {
      // Skip any runs before the first run specified
      if (currentRun < origEventID_.runID()) {
        fiIter_ = fileIndex_.findPosition(origEventID_.runID(), false);
        return getNextEntryTypeWanted();
      }
      return FileIndex::kRun;
    }
    if (processingMode_ == InputSource::Runs) {
      fiIter_ = fileIndex_.findPosition(currentRun.isValid() ?
                                        currentRun.next() : currentRun, false);
      return getNextEntryTypeWanted();
    }
    SubRunID const& currentSubRun = fiIter_->eventID_.subRunID();
    if (entryType == FileIndex::kSubRun) {
      // Skip any subRuns before the first subRun specified
      if ((currentRun == origEventID_.runID()) &&
          (currentSubRun < origEventID_.subRunID())) {
        fiIter_ = fileIndex_.findSubRunOrRunPosition(origEventID_.subRunID());
        return getNextEntryTypeWanted();
      }
      return FileIndex::kSubRun;
    }
    if (processingMode_ == InputSource::RunsAndSubRuns) {
      fiIter_ = fileIndex_.findSubRunOrRunPosition(currentSubRun.next());
      return getNextEntryTypeWanted();
    }
    assert(entryType == FileIndex::kEvent);
    // Skip any events before the first event specified
    if (fiIter_->eventID_ < origEventID_) {
      fiIter_ = fileIndex_.findPosition(origEventID_);
      return getNextEntryTypeWanted();
    }
    if (duplicateChecker_.get() &&
        duplicateChecker_->isDuplicateAndCheckActive(fiIter_->eventID_, fileName_)) {
      nextEntry();
      return getNextEntryTypeWanted();
    }
    if (eventsToSkip_ == 0) {
      return FileIndex::kEvent;
    }
    // We have specified a count of events to skip, keep skipping
    // events in this subRun block until we reach the end of the
    // subRun block or the full count of the number of events to skip.
    while ((eventsToSkip_ != 0) && (fiIter_ != fiEnd_) &&
           (getEntryType() == FileIndex::kEvent)) {
      nextEntry();
      --eventsToSkip_;
      while ((eventsToSkip_ != 0) && (fiIter_ != fiEnd_) &&
             (fiIter_->getEntryType() == FileIndex::kEvent) &&
             duplicateChecker_.get() &&
             duplicateChecker_->isDuplicateAndCheckActive(fiIter_->eventID_, fileName_)) {
        nextEntry();
      }
    }
    return getNextEntryTypeWanted();
  }

  void
  RootInputFile::
  validateFile()
  {
    if (!fileFormatVersion_.isValid()) {
      fileFormatVersion_.value_ = 0;
    }
    if (!eventTree().isValid()) {
      throw art::Exception{errors::DataCorruption}
      << "'Events' tree is corrupted or not present\n"
           << "in the input file.\n";
    }
    if (fileIndex_.empty()) {
      throw art::Exception{art::errors::FileReadError}
      << "FileIndex information is missing for the input file.\n";
    }
  }

  void
  RootInputFile::
  reportOpened()
  {
  }

  void
  RootInputFile::
  close(bool reallyClose)
  {
    if (!reallyClose) {
      return;
    }
    filePtr_->Close();
    for (auto const& sf : secondaryFiles_) {
      if (!sf) {
        continue;
      }
      sf->filePtr_->Close();
    }
  }

  void
  RootInputFile::
  fillHistory(EntryNumber const entry, History& history)
  {
    // We could consider doing delayed reading, but because we have to
    // store this History object in a different tree than the event
    // data tree, this is too hard to do in this first version.
    auto pHistory = &history;
    auto eventHistoryBranch = eventHistoryTree_->GetBranch(rootNames::eventHistoryBranchName().c_str());
    if (!eventHistoryBranch) {
      throw art::Exception{errors::DataCorruption}
      << "Failed to find history branch in event history tree.\n";
    }
    eventHistoryBranch->SetAddress(&pHistory);
    input::getEntry(eventHistoryTree_, entry);
  }

  int
  RootInputFile::
  skipEvents(int offset)
  {
    while ((offset > 0) && (fiIter_ != fiEnd_)) {
      if (fiIter_->getEntryType() == FileIndex::kEvent) {
        --offset;
      }
      nextEntry();
    }
    while ((offset < 0) && (fiIter_ != fiBegin_)) {
      previousEntry();
      if (fiIter_->getEntryType() == FileIndex::kEvent) {
        ++offset;
      }
    }
    while ((fiIter_ != fiEnd_) &&
           (fiIter_->getEntryType() != FileIndex::kEvent)) {
      nextEntry();
    }
    return offset;
  }

  // readEvent() is responsible for creating, and setting up, the
  // EventPrincipal.
  //
  //   1. create an EventPrincipal with a unique EventID
  //   2. For each entry in the provenance, put in one Group,
  //      holding the Provenance for the corresponding EDProduct.
  //   3. set up the caches in the EventPrincipal to know about this
  //      Group.
  //
  // We do *not* create the EDProduct instance (the equivalent of
  // reading the branch containing this EDProduct). That will be done
  // by the Delayed Reader, when it is asked to do so.
  //
  unique_ptr<EventPrincipal>
  RootInputFile::
  readEvent()
  {
    assert(fiIter_ != fiEnd_);
    assert(fiIter_->getEntryType() == FileIndex::kEvent);
    assert(fiIter_->eventID_.runID().isValid());
    auto const& entryNumbers = getEntryNumbers(InEvent);
    auto ep = readCurrentEvent(entryNumbers);
    assert(ep);
    assert(eventAux_.run() == fiIter_->eventID_.run() + forcedRunOffset_);
    assert(eventAux_.subRunID() == fiIter_->eventID_.subRunID());
    nextEntry();
    return move(ep);
  }

  // Reads event at the current entry in the tree.
  // Note: This function neither uses nor sets fiIter_.
  unique_ptr<EventPrincipal>
  RootInputFile::
  readCurrentEvent(pair<EntryNumbers, bool> const& entryNumbers)
  {
    assert(entryNumbers.first.size() == 1ull);
    fillAuxiliary_Event(entryNumbers.first.front());
    assert(eventAux_.eventID() == fiIter_->eventID_);
    unique_ptr<History> history = make_unique<History>();
    fillHistory(entryNumbers.first.front(), *history);
    overrideRunNumber(const_cast<EventID&>(eventAux_.eventID()), eventAux_.isRealData());
    auto ep = make_unique<EventPrincipal>(eventAux_,
                                          processConfiguration_,
                                          productListHolder_.productList_,
                                          &presentProducts_.get(InEvent),
                                          move(history),
                                          make_unique<RootDelayedReader>(fileFormatVersion_,
                                                                         nullptr,
                                                                         entryNumbers.first,
                                                                         &eventTree().branches(),
                                                                         eventTree().productProvenanceBranch(),
                                                                         saveMemoryObjectThreshold_,
                                                                         this,
                                                                         branchIDLists_.get(),
                                                                         InEvent,
                                                                         eventAux_.eventID()),
                                          entryNumbers.second);
    if (!delayedReadEventProducts_) {
      ep->readImmediate();
    }
    primaryEP_ = make_exempt_ptr(ep.get());
    return move(ep);
  }


  bool
  RootInputFile::
  readEventForSecondaryFile(EventID eID)
  {
    // Used just after opening a new secondary file in response to a
    // failed product lookup.  Synchronize the file index to the event
    // needed and create a secondary EventPrincipal for it.
    if (!setEntry_Event(eID, /*exact=*/true)) {
      // Error, could not find specified event in file.
      return false;
    }
    auto const& entryNumbers = getEntryNumbers(InEvent);
    assert(entryNumbers.first.size() == 1ull);
    fillAuxiliary_Event(entryNumbers.first.front());
    unique_ptr<History> history = make_unique<History>();
    fillHistory(entryNumbers.first.front(), *history);
    overrideRunNumber(const_cast<EventID&>(eventAux_.eventID()),
                      eventAux_.isRealData());
    auto ep = make_unique<EventPrincipal>(eventAux_,
                                          processConfiguration_,
                                          productListHolder_.productList_,
                                          &presentProducts_.get(InEvent),
                                          move(history),
                                          make_unique<RootDelayedReader>(fileFormatVersion_,
                                                                         nullptr,
                                                                         entryNumbers.first,
                                                                         &eventTree().branches(),
                                                                         eventTree().productProvenanceBranch(),
                                                                         saveMemoryObjectThreshold_,
                                                                         this,
                                                                         branchIDLists_.get(),
                                                                         InEvent,
                                                                         eventAux_.eventID()),
                                          entryNumbers.second);
    primaryFile_->primaryEP_->addSecondaryPrincipal(move(ep));
    return true;
  }

  unique_ptr<RangeSetHandler>
  RootInputFile::runRangeSetHandler()
  {
    return move(runRangeSetHandler_);
  }

  unique_ptr<RunPrincipal>
  RootInputFile::
  readRun()
  {
    assert(fiIter_ != fiEnd_);
    assert(fiIter_->getEntryType() == FileIndex::kRun);
    assert(fiIter_->eventID_.runID().isValid());
    auto const& entryNumbers = getEntryNumbers(InRun).first;
    auto rp = readCurrentRun(entryNumbers);
    advanceEntry(entryNumbers.size());
    return move(rp);
  }

  unique_ptr<RunPrincipal>
  RootInputFile::
  readCurrentRun(EntryNumbers const& entryNumbers)
  {
    runRangeSetHandler_ = fillAuxiliary_Run(entryNumbers);
    assert(runAux_.runID() == fiIter_->eventID_.runID());
    overrideRunNumber(runAux_);
    if (runAux_.beginTime() == Timestamp::invalidTimestamp()) {
      runAux_.beginTime(eventAux_.time());
      runAux_.endTime(Timestamp::invalidTimestamp());
    }
    auto rp = make_unique<RunPrincipal>(runAux_,
                                        processConfiguration_,
                                        productListHolder_.productList_,
                                        &presentProducts_.get(InRun),
                                        make_unique<RootDelayedReader>(fileFormatVersion_,
                                                                       sqliteDB_,
                                                                       entryNumbers,
                                                                       &runTree().branches(),
                                                                       runTree().productProvenanceBranch(),
                                                                       saveMemoryObjectThreshold_,
                                                                       this,
                                                                       nullptr,
                                                                       InRun,
                                                                       fiIter_->eventID_));
    if (!delayedReadRunProducts_) {
      rp->readImmediate();
    }
    primaryRP_ = make_exempt_ptr(rp.get());
    return move(rp);
  }

  bool
  RootInputFile::
  readRunForSecondaryFile(RunID rID)
  {
    // Used just after opening a new secondary file in response to a failed
    // product lookup.  Synchronize the file index to the run needed and
    // create a secondary RunPrincipal for it.
    if (!setEntry_Run(rID)) {
      // Error, could not find specified run in file.
      return false;
    }
    auto const& entryNumbers = getEntryNumbers(InRun).first;
    assert(fiIter_ != fiEnd_);
    assert(fiIter_->getEntryType() == FileIndex::kRun);
    assert(fiIter_->eventID_.runID().isValid());
    runRangeSetHandler_ = fillAuxiliary_Run(entryNumbers);
    assert(runAux_.runID() == fiIter_->eventID_.runID());
    overrideRunNumber(runAux_);
    if (runAux_.beginTime() == Timestamp::invalidTimestamp()) {
      runAux_.beginTime(eventAux_.time());
      runAux_.endTime(Timestamp::invalidTimestamp());
    }
    auto rp = make_unique<RunPrincipal>(runAux_,
                                        processConfiguration_,
                                        productListHolder_.productList_,
                                        &presentProducts_.get(InRun),
                                        make_unique<RootDelayedReader>(fileFormatVersion_,
                                                                       sqliteDB_,
                                                                       entryNumbers,
                                                                       &runTree().branches(),
                                                                       runTree().productProvenanceBranch(),
                                                                       saveMemoryObjectThreshold_,
                                                                       this,
                                                                       nullptr,
                                                                       InRun,
                                                                       fiIter_->eventID_));
    if (!delayedReadRunProducts_) {
      rp->readImmediate();
    }
    primaryFile_->primaryRP_->addSecondaryPrincipal(move(rp));
    return true;
  }

  unique_ptr<RangeSetHandler>
  RootInputFile::subRunRangeSetHandler()
  {
    return move(subRunRangeSetHandler_);
  }

  unique_ptr<SubRunPrincipal>
  RootInputFile::
  readSubRun(cet::exempt_ptr<RunPrincipal const> rp)
  {
    assert(fiIter_ != fiEnd_);
    assert(fiIter_->getEntryType() == FileIndex::kSubRun);
    auto const& entryNumbers = getEntryNumbers(InSubRun).first;
    auto srp = readCurrentSubRun(entryNumbers, rp);
    advanceEntry(entryNumbers.size());
    return move(srp);
  }

  unique_ptr<SubRunPrincipal>
  RootInputFile::
  readCurrentSubRun(EntryNumbers const& entryNumbers, cet::exempt_ptr<RunPrincipal const> rp [[gnu::unused]])
  {
    subRunRangeSetHandler_ = fillAuxiliary_SubRun(entryNumbers);
    assert(subRunAux_.subRunID() == fiIter_->eventID_.subRunID());
    overrideRunNumber(subRunAux_.id_);
    assert(subRunAux_.runID() == rp->runID());
    if (subRunAux_.beginTime() == Timestamp::invalidTimestamp()) {
      subRunAux_.beginTime_ = eventAux_.time();
      subRunAux_.endTime_ = Timestamp::invalidTimestamp();
    }
    auto srp = make_unique<SubRunPrincipal>(subRunAux_,
                                            processConfiguration_,
                                            productListHolder_.productList_,
                                            &presentProducts_.get(InSubRun),
                                            make_unique<RootDelayedReader>(fileFormatVersion_,
                                                                           sqliteDB_,
                                                                           entryNumbers,
                                                                           &subRunTree().branches(),
                                                                           subRunTree().productProvenanceBranch(),
                                                                           saveMemoryObjectThreshold_,
                                                                           this,
                                                                           nullptr,
                                                                           InSubRun,
                                                                           fiIter_->eventID_));
    if (!delayedReadSubRunProducts_) {
      srp->readImmediate();
    }
    primarySRP_ = make_exempt_ptr(srp.get());
    return move(srp);
  }

  bool
  RootInputFile::
  readSubRunForSecondaryFile(SubRunID srID)
  {
    // Used just after opening a new secondary file in response to a failed
    // product lookup.  Synchronize the file index to the subRun needed and
    // create a secondary SubRunPrincipal for it.
    if (!setEntry_SubRun(srID)) {
      // Error, could not find specified subRun in file.
      return false;
    }
    auto const& entryNumbers = getEntryNumbers(InSubRun).first;
    assert(fiIter_ != fiEnd_);
    assert(fiIter_->getEntryType() == FileIndex::kSubRun);
    subRunRangeSetHandler_ = fillAuxiliary_SubRun(entryNumbers);
    assert(subRunAux_.subRunID() == fiIter_->eventID_.subRunID());
    overrideRunNumber(subRunAux_.id_);
    if (subRunAux_.beginTime() == Timestamp::invalidTimestamp()) {
      subRunAux_.beginTime_ = eventAux_.time();
      subRunAux_.endTime_ = Timestamp::invalidTimestamp();
    }
    auto srp = make_unique<SubRunPrincipal>(subRunAux_,
                                            processConfiguration_,
                                            productListHolder_.productList_,
                                            &presentProducts_.get(InSubRun),
                                            make_unique<RootDelayedReader>(fileFormatVersion_,
                                                                           sqliteDB_,
                                                                           entryNumbers,
                                                                           &subRunTree().branches(),
                                                                           subRunTree().productProvenanceBranch(),
                                                                           saveMemoryObjectThreshold_,
                                                                           this,
                                                                           nullptr,
                                                                           InSubRun,
                                                                           fiIter_->eventID_));
    if (!delayedReadSubRunProducts_) {
      srp->readImmediate();
    }
    primaryFile_->primarySRP_->addSecondaryPrincipal(move(srp));
    return true;
  }

  void
  RootInputFile::
  overrideRunNumber(RunAuxiliary& aux)
  {
    if (forcedRunOffset_ != 0) {
      aux.runID(RunID(aux.run() + forcedRunOffset_));
    }
    if (aux.runID() < RunID::firstRun()) {
      aux.runID(RunID::firstRun());
    }
  }

  void
  RootInputFile::
  overrideRunNumber(SubRunID& id)
  {
    if (forcedRunOffset_ != 0) {
      id = SubRunID(id.run() + forcedRunOffset_, id.subRun());
    }
  }

  void
  RootInputFile::
  overrideRunNumber(EventID& id, bool isRealData)
  {
    if (forcedRunOffset_ == 0) {
      return;
    }
    if (isRealData) {
      throw art::Exception{errors::Configuration, "RootInputFile::overrideRunNumber()"}
      << "The 'setRunNumber' parameter of RootInput cannot "
           << "be used with real data.\n";
    }
    id = EventID(id.run() + forcedRunOffset_, id.subRun(), id.event());
  }

  void
  RootInputFile::
  readEventHistoryTree(unsigned int treeCacheSize)
  {
    // Read in the event history tree, if we have one...
    eventHistoryTree_ = static_cast<TTree*>(filePtr_->Get(rootNames::eventHistoryTreeName().c_str()));
    if (!eventHistoryTree_) {
      throw art::Exception{errors::DataCorruption}
      << "Failed to find the event history tree.\n";
    }
    eventHistoryTree_->SetCacheSize(static_cast<Long64_t>(treeCacheSize));
  }

  void
  RootInputFile::
  initializeDuplicateChecker()
  {
    if (duplicateChecker_.get() == nullptr) {
      return;
    }
    if (eventTree().entries()) {
      //FIXME: We don't initialize the duplicate checker if there are no events!
      fillAuxiliary_Event(0);
      duplicateChecker_->init(eventAux_.isRealData(), fileIndex_);
    }
  }

  pair<RootInputFile::EntryNumbers, bool>
  RootInputFile::
  getEntryNumbers(BranchType const bt)
  {
    EntryNumbers enumbers;
    if (fiIter_ == fiEnd_) {
      return pair<EntryNumbers, bool>{enumbers, true};
    }
    auto const eid = fiIter_->eventID_;
    auto iter = fiIter_;
    for (; (iter != fiEnd_) && (iter->eventID_ == eid); ++iter) {
      enumbers.push_back(iter->entry_);
    }
    if ((bt == InEvent) && (enumbers.size() > 1ul)) {
      throw Exception{errors::FileReadError}
      << "File " << fileName_ << " has multiple entries for\n" << eid << '\n';
    }
    bool const lastInSubRun{(iter == fiEnd_) || (iter->eventID_.subRun() != eid.subRun())};
    return pair<EntryNumbers, bool>{enumbers, lastInSubRun};
  }

  std::array<AvailableProducts_t, NumBranchTypes>
  RootInputFile::
  fillPerBranchTypePresenceFlags(ProductList const& prodList)
  {
    std::array<AvailableProducts_t, NumBranchTypes> result;
    for (auto const& bk_and_bd : prodList) {
      auto const& bd = bk_and_bd.second;
      auto const bt = bd.branchType();
      auto const& pid = bd.productID();
      if (treePointers_[bt]->tree()->GetBranch(bd.branchName().c_str()) != nullptr) {
        result[bt].emplace(pid);
      }
    }
    return result;
  }

  void
  RootInputFile::
  dropOnInput(GroupSelectorRules const& rules,
              bool const dropDescendants,
              ProductList& prodList)
  {
    // This is the selector for drop on input.
    GroupSelector const groupSelector{rules, prodList};
    // Do drop on input. On the first pass, just fill in a set of
    // branches to be dropped.  Use the BranchChildren class to
    // assemble list of children to drop.
    BranchChildren children;
    set<ProductID> branchesToDrop;
    for (auto const& prod : prodList) {
      auto const& pd = prod.second;
      if (!groupSelector.selected(pd)) {
        if (dropDescendants) {
          children.appendToDescendants(pd.productID(), branchesToDrop);
        }
        else {
          branchesToDrop.insert(pd.productID());
        }
      }
    }

    // On this pass, actually drop the branches.
    auto branchesToDropEnd = branchesToDrop.cend();
    for (auto I = prodList.begin(), E = prodList.end(); I != E;) {
      auto const& bd = I->second;
      bool drop = branchesToDrop.find(bd.productID()) != branchesToDropEnd;
      if (!drop) {
        ++I;
        checkDictionaries(bd);
        continue;
      }
      if (groupSelector.selected(bd)) {
        mf::LogWarning("RootInputFile")
          << "Branch '"
          << bd.branchName()
          << "' is being dropped from the input\n"
          << "of file '"
          << fileName_
          << "' because it is dependent on a branch\n"
          << "that was explicitly dropped.\n";
      }
      treePointers_[bd.branchType()]->dropBranch(bd.branchName());
      auto icopy = I++;
      prodList.erase(icopy);
    }
  }

  void
  RootInputFile::
  openSecondaryFile(int const idx)
  {
    secondaryFiles_[idx] = rifSequence_->openSecondaryFile(secondaryFileNames_[idx], this);
  }

  unique_ptr<art::ResultsPrincipal>
  RootInputFile::
  readResults()
  {
    unique_ptr<art::ResultsPrincipal> resp;
    if (!resultsTree().isValid()) {
      resp = make_unique<ResultsPrincipal>(ResultsAuxiliary{},
                                           processConfiguration_,
                                           ProductList{},
                                           nullptr);
      return resp;
    }

    EntryNumbers const& entryNumbers {0}; // FIXME: Not sure hard-coding 0 is the right thing to do.
    assert(entryNumbers.size() == 1ull);
    fillAuxiliary_Results(entryNumbers.front());
    resp = std::make_unique<ResultsPrincipal>(resultsAux_,
                                              processConfiguration_,
                                              productListHolder_.productList_,
                                              &presentProducts_.get(InResults),
                                              make_unique<RootDelayedReader>(fileFormatVersion_,
                                                                             nullptr,
                                                                             entryNumbers,
                                                                             &resultsTree().branches(),
                                                                             resultsTree().productProvenanceBranch(),
                                                                             saveMemoryObjectThreshold_,
                                                                             this,
                                                                             nullptr,
                                                                             InResults,
                                                                             EventID{}));
    return move(resp);
  }

} // namespace art
