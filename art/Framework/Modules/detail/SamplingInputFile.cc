#include "art/Framework/Modules/detail/SamplingInputFile.h"

#include "art/Framework/Core/GroupSelector.h"
#include "art/Framework/IO/Root/BranchMapperWithReader.h"
#include "art/Framework/IO/Root/RootDB/TKeyVFSOpenPolicy.h"
#include "art/Framework/IO/Root/RootDB/have_table.h"
#include "art/Framework/IO/Root/checkDictionaries.h"
#include "art/Framework/IO/Root/detail/dropBranch.h"
#include "art/Framework/IO/Root/detail/readFileIndex.h"
#include "art/Framework/IO/Root/detail/readMetadata.h"
#include "art/Framework/IO/Root/rootErrMsgs.h"
#include "art/Framework/Modules/detail/SamplingDelayedReader.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/DatabaseConnection.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/BranchChildren.h"
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
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <set>
#include <string>

namespace detail = art::detail;

using EntriesForID_t = detail::SamplingInputFile::EntriesForID_t;
using ProductsForKey_t = detail::SamplingInputFile::ProductsForKey_t;
using namespace std::string_literals;

namespace {
  TTree*
  get_tree(TFile& file, std::string const& treeName)
  {
    auto result = dynamic_cast<TTree*>(file.Get(treeName.c_str()));
    if (result == nullptr) {
      throw art::Exception{art::errors::FileReadError,
                           "An error occurred while trying to read "s +
                             file.GetName()}
        << art::couldNotFindTree(treeName);
    }
    return result;
  }

  TTree*
  get_tree(TFile& file,
           std::string const& treeName,
           unsigned int const treeCacheSize,
           int64_t const treeMaxVirtualSize)
  {
    auto result = get_tree(file, treeName);
    result->SetCacheSize(static_cast<Long64_t>(treeCacheSize));
    if (treeMaxVirtualSize >= 0) {
      result->SetMaxVirtualSize(static_cast<Long64_t>(treeMaxVirtualSize));
    }
    return result;
  }
}

detail::SamplingInputFile::SamplingInputFile(
  std::string const& dataset,
  std::string const& filename,
  double const weight,
  double const probability,
  EventID const& firstEvent,
  GroupSelectorRules const& groupSelectorRules,
  bool const dropDescendants,
  unsigned int const treeCacheSize,
  int64_t const treeMaxVirtualSize,
  int64_t const saveMemoryObjectThreshold,
  BranchDescription const& sampledEventInfoDesc,
  bool const compactRangeSets,
  std::map<BranchKey, BranchDescription>& oldKeyToSampledProductDescription,
  ModuleDescription const& md,
  bool const readIncomingParameterSets,
  MasterProductRegistry& mpr)
  : dataset_{dataset}
  , file_{std::make_unique<TFile>(filename.c_str())}
  , weight_{weight}
  , probability_{probability}
  , firstEvent_{firstEvent}
  , saveMemoryObjectThreshold_{saveMemoryObjectThreshold}
  , sampledEventInfoDesc_{sampledEventInfoDesc}
  , compactRangeSets_{compactRangeSets}
{
  auto metaDataTree = get_tree(*file_, rootNames::metaDataTreeName());

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

  // Event-level trees
  eventHistoryTree_ = get_tree(*file_, rootNames::eventHistoryTreeName());
  {
    auto eventHistoryBranch =
      eventHistoryTree_->GetBranch(rootNames::eventHistoryBranchName().c_str());
    eventHistoryBranch->SetAddress(nullptr);
  }
  eventTree_ = get_tree(*file_,
                        BranchTypeToProductTreeName(InEvent),
                        treeCacheSize,
                        treeMaxVirtualSize);
  auxBranch_ =
    eventTree_->GetBranch(BranchTypeToAuxiliaryBranchName(InEvent).c_str());
  auxBranch_->SetAddress(nullptr);

  eventMetaTree_ = get_tree(*file_, BranchTypeToMetaDataTreeName(InEvent));
  productProvenanceBranch_ =
    eventMetaTree_->GetBranch(productProvenanceBranchName(InEvent).c_str());
  productProvenanceBranch_->SetAddress(nullptr);

  // Higher-level trees
  subRunTree_ = get_tree(*file_,
                         BranchTypeToProductTreeName(InSubRun),
                         treeCacheSize,
                         treeMaxVirtualSize);
  runTree_ = get_tree(*file_,
                      BranchTypeToProductTreeName(InRun),
                      treeCacheSize,
                      treeMaxVirtualSize);

  // Read the BranchChildren, necessary for dropping descendent products
  auto const branchChildren =
    detail::readMetadata<BranchChildren>(metaDataTree);

  // Read the ProductList
  productListHolder_ = detail::readMetadata<ProductRegistry>(metaDataTree);
  auto& productList = productListHolder_.productList_;
  dropOnInput_(
    groupSelectorRules, branchChildren, dropDescendants, productList);

  AvailableProducts_t availableEventProducts{};
  for (auto& pr : productList) {
    auto const& key = pr.first;
    auto& pd = pr.second;
    auto const bt = pd.branchType();
    auto branch = treeForBranchType_(bt)->GetBranch(pd.branchName().c_str());
    branch->SetAddress(nullptr);
    if (bt == InSubRun || bt == InRun) {
      std::string const wrapped_product{"art::Sampled<" +
                                        pd.producedClassName() + ">"};
      ProcessConfiguration const pc{"SampledFrom" + pd.processName(),
                                    md.parameterSetID(),
                                    md.releaseVersion()};
      BranchDescription sampledDesc{bt,
                                    pd.moduleLabel(),
                                    pc.processName(),
                                    uniform_type_name(wrapped_product),
                                    pd.productInstanceName(),
                                    pc.parameterSetID(),
                                    pc.id(),
                                    false,
                                    false};
      oldKeyToSampledProductDescription.emplace(key, std::move(sampledDesc));
    } else {
      bool const present{branch != nullptr};
      if (present) {
        availableEventProducts.emplace(pd.productID());
      }

      auto const validity = present ?
                              BranchDescription::Transients::PresentFromSource :
                              BranchDescription::Transients::Dropped;
      pd.setValidity(validity);
    }

    branches_.emplace(pr.first, input::BranchInfo{pd, branch});
  }

  mpr.updateFromInputFile(productList);

  // Register newly created data product
  availableEventProducts.emplace(sampledEventInfoDesc_.productID());
  checkDictionaries(sampledEventInfoDesc_);
  productList.emplace(BranchKey{sampledEventInfoDesc_}, sampledEventInfoDesc_);

  presentEventProducts_ = ProductTable{
    make_product_descriptions(productList), InEvent, availableEventProducts};
}

bool
detail::SamplingInputFile::updateEventEntry_(FileIndex::const_iterator& it,
                                             input::EntryNumber& entry) const
{
  for (; it != fiEnd_; ++it) {
    if (it->getEntryType() != art::FileIndex::kEvent ||
        it->eventID_ < firstEvent_) {
      continue;
    }

    entry = it->entry_;
    return true;
  }

  return false;
}

art::EventID
detail::SamplingInputFile::nextEvent() const
{
  auto local_it = fiIter_;
  input::EntryNumber entry;
  return updateEventEntry_(local_it, entry) ? local_it->eventID_ :
                                              EventID::invalidEvent();
}

bool
detail::SamplingInputFile::readyForNextEvent()
{
  bool const another_one = updateEventEntry_(fiIter_, currentEventEntry_);
  if (another_one) {
    ++fiIter_;
  }
  return another_one;
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

ProductsForKey_t
detail::SamplingInputFile::productsFor(EntriesForID_t const& entries,
                                       BranchType const bt)
{
  ProductsForKey_t result;
  for (auto const& pr : entries) {
    auto const& id = pr.first;
    auto const& tree_entries = pr.second;

    SamplingDelayedReader const reader{fileFormatVersion_,
                                       sqliteDB_,
                                       tree_entries,
                                       branches_,
                                       treeForBranchType_(bt),
                                       saveMemoryObjectThreshold_,
                                       branchIDLists_.get(),
                                       bt,
                                       id,
                                       compactRangeSets_};
    for (auto const& pr : productListHolder_.productList_) {
      auto const& key = pr.first;
      auto const& bd = pr.second;
      if (bd.branchType() != bt)
        continue;
      RangeSet rs{RangeSet::invalid()};
      auto const class_name = uniform_type_name(bd.producedClassName());
      auto product = reader.getProduct(key, wrappedClassName(class_name), rs);
      result[key].emplace(id.subRunID(), move(product));
    }
  }
  return result;
}

std::unique_ptr<art::EventPrincipal>
detail::SamplingInputFile::readEvent(EventID const& eventID,
                                     ProcessConfigurations const& sampled_pcs,
                                     ProcessConfiguration const& current_pc)
{
  auto const on_disk_aux = auxiliaryForEntry_(currentEventEntry_);
  auto history = historyForEntry_(currentEventEntry_);

  ProcessHistory ph;
  bool const found [[gnu::unused]]{
    ProcessHistoryRegistry::get(history.processHistoryID(), ph)};
  assert(found);

  for (auto const& sampled_pc : sampled_pcs) {
    ph.push_back(sampled_pc);
  }
  auto const id = ph.id();
  ProcessHistoryRegistry::emplace(id, ph);
  history.setProcessHistoryID(id);

  // We do *not* keep the on-disk EventID for the primary event; we
  // instead create it as an event product.
  art::EventAuxiliary const aux{eventID,
                                on_disk_aux.time(),
                                on_disk_aux.isRealData(),
                                on_disk_aux.experimentType()};
  auto const on_disk_id = on_disk_aux.id();
  auto ep = std::make_unique<art::EventPrincipal>(
    aux,
    current_pc,
    &presentEventProducts_,
    true,
    true,
    std::make_shared<History>(std::move(history)),
    std::make_unique<BranchMapperWithReader>(productProvenanceBranch_,
                                             currentEventEntry_),
    std::make_unique<SamplingDelayedReader>(
      fileFormatVersion_,
      sqliteDB_,
      input::EntryNumbers{currentEventEntry_},
      branches_,
      eventTree_,
      -1 /* saveMemoryObjectThreshold */,
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

void
detail::SamplingInputFile::dropOnInput_(GroupSelectorRules const& rules,
                                        BranchChildren const& children,
                                        bool const dropDescendants,
                                        ProductList& prodList)
{
  // FIXME: The functionality below is a near duplicate to that
  //        provided in RootInput.

  GroupSelector const groupSelector{rules, prodList};
  // Do drop on input. On the first pass, just fill in a set of
  // branches to be dropped.

  // FIXME: ProductID does not include BranchType, so this algorithm
  //        may be problematic.
  std::set<ProductID> branchesToDrop;
  for (auto const& prod : prodList) {
    auto const& pd = prod.second;
    // We explicitly do not support results products for the Sampling
    // input source.
    if (pd.branchType() == InResults || !groupSelector.selected(pd)) {
      if (dropDescendants) {
        children.appendToDescendants(pd.productID(), branchesToDrop);
      } else {
        branchesToDrop.insert(pd.productID());
      }
    }
  }
  // On this pass, actually drop the branches.
  auto branchesToDropEnd = branchesToDrop.cend();
  for (auto I = prodList.begin(), E = prodList.end(); I != E;) {
    auto const& pd = I->second;
    bool drop = branchesToDrop.find(pd.productID()) != branchesToDropEnd;
    if (!drop) {
      ++I;
      checkDictionaries(pd);
      continue;
    }
    if (groupSelector.selected(pd)) {
      mf::LogWarning("SamplingInputFile")
        << "Branch '" << pd.branchName()
        << "' is being dropped from the input\n"
        << "of file '" << file_->GetName()
        << "' because it is dependent on a branch\n"
        << "that was explicitly dropped.\n";
    }
    dropBranch(treeForBranchType_(pd.branchType()), pd.branchName());
    auto icopy = I++;
    prodList.erase(icopy);
  }
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
