#include "art/Framework/Modules/detail/SamplingInputFile.h"

#include "art/Framework/IO/Root/BranchMapperWithReader.h"
#include "art/Framework/IO/Root/RootDB/TKeyVFSOpenPolicy.h"
#include "art/Framework/IO/Root/RootDB/have_table.h"
#include "art/Framework/IO/Root/checkDictionaries.h"
#include "art/Framework/IO/Root/detail/readFileIndex.h"
#include "art/Framework/IO/Root/detail/readMetadata.h"
#include "art/Framework/IO/Root/rootErrMsgs.h"
#include "art/Framework/Modules/detail/SamplingDelayedReader.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/DatabaseConnection.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/SampledInfo.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "canvas/Utilities/uniform_type_name.h"
#include "canvas_root_io/Streamers/ProductIDStreamer.h"
#include "fhiclcpp/ParameterSetRegistry.h"

namespace detail = art::detail;

using EntriesForID_t = detail::SamplingInputFile::EntriesForID_t;
using Products_t = detail::SamplingInputFile::Products_t;

detail::SamplingInputFile::SamplingInputFile(
  std::string const& dataset,
  std::string const& filename,
  double const weight,
  double const probability,
  BranchDescription const& sampledEventInfoDesc,
  std::map<BranchKey, BranchDescription>& oldKeyToSampledProductDescription,
  ModuleDescription const& md,
  bool const readIncomingParameterSets,
  MasterProductRegistry& mpr)
  : dataset_{dataset}
  , file_{std::make_unique<TFile>(filename.c_str())}
  , weight_{weight}
  , probability_{probability}
  , sampledEventInfoDesc_{sampledEventInfoDesc}
{
  // Read metadata tree
  auto metaDataTree =
    dynamic_cast<TTree*>(file_->Get(rootNames::metaDataTreeName().c_str()));
  if (!metaDataTree) {
    throw Exception{errors::FileReadError}
      << couldNotFindTree(rootNames::metaDataTreeName());
  }

  // Read the ProcessHistory
  {
    auto pHistMap = detail::readMetadata<ProcessHistoryMap>(metaDataTree);
    ProcessHistoryRegistry::put(pHistMap);
  }

  // Read file format version
  fileFormatVersion_ = detail::readMetadata<FileFormatVersion>(metaDataTree);

  // Also need to check RootFileDB if we have one.
  if (fileFormatVersion_.value_ >= 5) {
    sqliteDB_ = ServiceHandle<DatabaseConnection> {}
    ->get<TKeyVFSOpenPolicy>("RootFileDB", file_.get());
    if (readIncomingParameterSets &&
        have_table(sqliteDB_, "ParameterSets", dataset_)) {
      fhicl::ParameterSetRegistry::importFrom(sqliteDB_);
    }
  }

  // Read file index
  auto findexPtr = &fileIndex_;
  detail::readFileIndex(file_.get(), metaDataTree, findexPtr);
  fiIter_ = fileIndex_.begin();
  fiEnd_ = fileIndex_.end();

  // To support files that contain BranchIDLists
  BranchIDLists branchIDLists{};
  if (detail::readMetadata(metaDataTree, branchIDLists)) {
    branchIDLists_ = std::make_unique<BranchIDLists>(std::move(branchIDLists));
    configureProductIDStreamer(branchIDLists_.get());
  }

  // Read event history tree
  eventHistoryTree_ =
    static_cast<TTree*>(file_->Get(rootNames::eventHistoryTreeName().c_str()));
  if (!eventHistoryTree_) {
    throw art::Exception{errors::DataCorruption}
      << "Failed to find the event history tree.\n";
  }
  // eventHistoryTree_->SetCacheSize(static_cast<Long64_t>(treeCacheSize));

  // Read event (meta)data trees
  eventTree_ = dynamic_cast<TTree*>(
    file_->Get(BranchTypeToProductTreeName(InEvent).c_str()));
  eventMetaTree_ = dynamic_cast<TTree*>(
    file_->Get(BranchTypeToMetaDataTreeName(InEvent).c_str()));

  if (eventTree_) {
    auxBranch_ =
      eventTree_->GetBranch(BranchTypeToAuxiliaryBranchName(InEvent).c_str());
  }
  if (eventMetaTree_) {
    productProvenanceBranch_ =
      eventMetaTree_->GetBranch(productProvenanceBranchName(InEvent).c_str());
  }

  // Read (sub)run data trees
  subRunTree_ = dynamic_cast<TTree*>(
    file_->Get(BranchTypeToProductTreeName(InSubRun).c_str()));
  runTree_ = dynamic_cast<TTree*>(
    file_->Get(BranchTypeToProductTreeName(InRun).c_str()));
  // Add checks for pointers above

  // Read the ProductList
  AvailableProducts_t availableEventProducts{};
  productListHolder_ = detail::readMetadata<ProductRegistry>(metaDataTree);
  auto& eventProductList = productListHolder_.productList_;
  //  dropOnInput(groupSelectorRules, branchChildren, dropDescendants,
  //  prodList);

  for (auto& pr : eventProductList) {
    auto const& key = pr.first;
    auto& pd = pr.second;
    auto const bt = pd.branchType();
    auto branch = treeForBranchType_(bt)->GetBranch(pd.branchName().c_str());
    if (bt == InSubRun || bt == InRun) {
      std::string const wrapped_product{"art::Sampled<" +
                                        pd.producedClassName() + ">"};
      BranchDescription sampledDesc{bt,
                                    pd.moduleLabel(),
                                    md.processName(),
                                    uniform_type_name(wrapped_product),
                                    pd.productInstanceName(),
                                    md.parameterSetID(),
                                    md.processConfigurationID(),
                                    false,
                                    false};
      oldKeyToSampledProductDescription.emplace(key, std::move(sampledDesc));
    } else {
      bool const present{branch != nullptr};
      if (present) {
        availableEventProducts.emplace(pd.productID());
        checkDictionaries(pd);
      }

      auto const validity = present ?
                              BranchDescription::Transients::PresentFromSource :
                              BranchDescription::Transients::Dropped;
      pd.setValidity(validity);
    }

    branches_.emplace(pr.first, input::BranchInfo{pd, branch});
  }

  mpr.updateFromInputFile(eventProductList);

  // Register newly created data product
  availableEventProducts.emplace(sampledEventInfoDesc_.productID());
  checkDictionaries(sampledEventInfoDesc_);
  eventProductList.emplace(BranchKey{sampledEventInfoDesc_},
                           sampledEventInfoDesc_);

  presentEventProducts_ =
    ProductTable{make_product_descriptions(eventProductList),
                 InEvent,
                 availableEventProducts};
}

bool
detail::SamplingInputFile::entryForNextEvent(art::input::EntryNumber& entry)
{
  while (fiIter_ != fiEnd_ && fiIter_->getEntryType() != FileIndex::kEvent) {
    ++fiIter_;
  }

  if (fiIter_ == fiEnd_) {
    return false;
  }

  entry = fiIter_->entry_;
  ++fiIter_;
  return true;
}

namespace art {
  namespace {
    constexpr FileIndex::EntryType
    to_entry_type(BranchType const bt)
    {
      switch (bt) {
        case InRun:
          return FileIndex::kRun;
        case InSubRun:
          return FileIndex::kSubRun;
        case InEvent:
          return FileIndex::kEvent;
        default:
          return FileIndex::kEnd;
      }
    }
  }
}

EntriesForID_t
detail::SamplingInputFile::treeEntries(BranchType const bt)
{
  EntriesForID_t entries;
  for (auto const& element : fileIndex_) {
    if (element.getEntryType() != to_entry_type(bt)) {
      continue;
    }
    entries[element.eventID_].push_back(element.entry_);
  }
  return entries;
}

Products_t
detail::SamplingInputFile::productsFor(EntriesForID_t const& entries,
                                       BranchType const bt)
{
  Products_t result;
  for (auto const& pr : entries) {
    auto const& id = pr.first;
    auto const& tree_entries = pr.second;

    SamplingDelayedReader const reader{fileFormatVersion_,
                                       sqliteDB_,
                                       tree_entries,
                                       branches_,
                                       treeForBranchType_(bt),
                                       -1 /* saveMemoryObjectThreshold */,
                                       branchIDLists_.get(),
                                       bt,
                                       id,
                                       false /* compact range sets */};
    for (auto const& pr : productListHolder_.productList_) {
      auto const& key = pr.first;
      auto const& bd = pr.second;
      if (bd.branchType() != bt)
        continue;
      RangeSet rs{RangeSet::invalid()};
      auto const class_name = uniform_type_name(bd.producedClassName());
      auto product = reader.getProduct(key, wrappedClassName(class_name), rs);
      result[key].push_back(move(product));
    }
  }
  return result;
}

std::unique_ptr<art::EventPrincipal>
detail::SamplingInputFile::readEvent(input::EntryNumber const entry,
                                     EventID const& eventID,
                                     ProcessConfiguration const& pc)
{
  auto const on_disk_aux = auxiliaryForEntry_(entry);
  auto history = historyForEntry_(entry);

  // We do *not* keep the on-disk EventID for the primary event; we
  // instead create it as an event product.
  art::EventAuxiliary const aux{eventID,
                                on_disk_aux.time(),
                                on_disk_aux.isRealData(),
                                on_disk_aux.experimentType()};
  auto const on_disk_id = on_disk_aux.id();
  auto ep = std::make_unique<art::EventPrincipal>(
    aux,
    pc,
    &presentEventProducts_,
    std::make_shared<History>(std::move(history)),
    std::make_unique<BranchMapperWithReader>(productProvenanceBranch_, entry),
    std::make_unique<SamplingDelayedReader>(fileFormatVersion_,
                                            sqliteDB_,
                                            input::EntryNumbers{entry},
                                            branches_,
                                            eventTree_,
                                            -1,
                                            branchIDLists_.get(),
                                            InEvent,
                                            on_disk_id,
                                            false));
  for (auto const& pr : branches_) {
    ep->fillGroup(pr.second.branchDescription_);
  }

  // Place sampled EventID onto event
  auto sampledEventID = std::make_unique<SampledEventInfo>(
    SampledEventInfo{on_disk_id, dataset_, weight_, probability_});
  auto wp = std::make_unique<Wrapper<SampledEventInfo>>(move(sampledEventID));
  auto const& pd = sampledEventInfoDesc_;
  ep->put(std::move(wp),
          pd,
          std::make_unique<ProductProvenance const>(pd.productID(),
                                                    productstatus::present()));
  return ep;
}

TTree*
detail::SamplingInputFile::treeForBranchType_(BranchType const bt) const
{
  switch (bt) {
    case InEvent:
      return eventTree_;
    case InSubRun:
      return subRunTree_;
    case InRun:
      return runTree_;
    default: {
      throw Exception{errors::LogicError}
        << "Cannot call treeForBranchType_ for a branch type of " << bt;
    }
  }
}

art::EventAuxiliary
detail::SamplingInputFile::auxiliaryForEntry_(input::EntryNumber const entry)
{
  auto aux = std::make_unique<EventAuxiliary>();
  auto pAux = aux.get();
  auxBranch_->SetAddress(&pAux);
  eventTree_->LoadTree(entry);
  input::getEntry(auxBranch_, entry);
  return *aux;
}

art::History
detail::SamplingInputFile::historyForEntry_(input::EntryNumber const entry)
{
  // We could consider doing delayed reading, but because we have to
  // store this History object in a different tree than the event
  // data tree, this is too hard to do in this first version.
  History history;
  auto pHistory = &history;
  auto eventHistoryBranch =
    eventHistoryTree_->GetBranch(rootNames::eventHistoryBranchName().c_str());
  if (!eventHistoryBranch) {
    throw art::Exception{errors::DataCorruption}
      << "Failed to find history branch in event history tree.\n";
  }
  eventHistoryBranch->SetAddress(&pHistory);
  input::getEntry(eventHistoryTree_, entry);
  return history;
}
