#include "art/Framework/IO/Root/RootIOPolicy.h"
#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/detail/readFileIndex.h"
#include "art/Framework/IO/Root/detail/readMetadata.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas_root_io/Streamers/ProductIDStreamer.h"
#include "canvas_root_io/Streamers/RefCoreStreamer.h"
#include "canvas_root_io/Utilities/DictionaryChecker.h"

namespace {
  std::array<cet::exempt_ptr<TTree>, art::NumBranchTypes>
  initDataTrees(cet::value_ptr<TFile> const& currentFile)
  {
    std::array<cet::exempt_ptr<TTree>, art::NumBranchTypes> result;
    for (auto bt = 0; bt != art::NumBranchTypes; ++bt) {
      result[bt].reset(static_cast<TTree*>(currentFile->Get(
        art::rootNames::dataTreeName(static_cast<art::BranchType>(bt))
          .c_str())));
      if (result[bt].get() == nullptr and bt != art::InResults) {
        throw art::Exception(art::errors::FileReadError)
          << "Unable to read event tree from secondary event stream file "
          << currentFile->GetName() << ".\n";
      }
    }
    return result;
  }
}

void
art::RootIOPolicy::openAndReadMetaData(std::string filename, MixOpList& mixOps)
{
  // Open file.
  try {
    // FIXME: threading: This is not thread-safe!!!
    currentFile_.reset(TFile::Open(filename.c_str()));
  }
  catch (std::exception const& e) {
    throw Exception(errors::FileOpenError, e.what())
      << "Unable to open specified secondary event stream file " << filename
      << ".\n";
  }
  if (!currentFile_ || currentFile_->IsZombie()) {
    throw Exception(errors::FileOpenError)
      << "Unable to open specified secondary event stream file " << filename
      << ".\n";
  }
  // Obtain meta data tree.
  currentMetaDataTree_.reset(static_cast<TTree*>(
    currentFile_->Get(rootNames::metaDataTreeName().c_str())));
  if (currentMetaDataTree_.get() == nullptr) {
    throw Exception(errors::FileReadError)
      << "Unable to read meta data tree from secondary event stream file "
      << filename << ".\n";
  }
  currentDataTrees_ = initDataTrees(currentFile_);
  auto nevents = currentDataTrees_[InEvent]->GetEntries();
  if (nevents < 0) {
    throw Exception(errors::FileReadError)
      << "Error when retrieving number of entries in event tree for file "
      << filename << ".\n";
  }
  assert(nevents >= 0);
  nEventsInCurrentFile_ = static_cast<std::size_t>(nevents);

  // Read file index
  FileIndex* fileIndexPtr = &fileIndexInCurrentFile_;
  detail::readFileIndex(
    currentFile_.get(), currentMetaDataTree_.get(), fileIndexPtr);

  // To support files that contain BranchIDLists
  BranchIDLists branchIDLists{};
  if (detail::readMetadata(currentMetaDataTree_.get(), branchIDLists)) {
    branchIDListsInCurrentFile_ =
      std::make_unique<BranchIDLists>(std::move(branchIDLists));
    configureProductIDStreamer(branchIDListsInCurrentFile_.get());
  }

  // Check file format era.
  ffVersion_ =
    detail::readMetadata<FileFormatVersion>(currentMetaDataTree_.get());

  std::string const expected_era = getFileFormatEra();
  if (ffVersion_.era_ != expected_era) {
    throw Exception(errors::FileReadError)
      << "Can only read files written during the \"" << expected_era
      << "\" era: "
      << "Era of "
      << "\"" << filename << "\" was "
      << (ffVersion_.era_.empty() ? "not set" :
                                    ("set to \"" + ffVersion_.era_ + "\" "))
      << ".\n";
  }
  auto dbCount = 0;
  for (auto const tree : currentDataTrees_) {
    if (tree.get()) {
      dataBranches_[dbCount].reset(tree.get());
    }
    ++dbCount;
  }

  branchInfos_.clear();
  root::DictionaryChecker checker{};
  for (auto& op : mixOps) {
    auto const bt = op->branchType();
    RootBranchInfo info{};
    auto const iType = op->inputType();
    auto const success =
      dataBranches_[bt].findBranchInfo(iType, op->inputTag(), info);
    if (!success) {
      throw Exception(errors::ProductNotFound)
        << "Unable to find requested product " << op->inputTag() << " of type "
        << iType.friendlyClassName() << " in secondary input stream.\n";
    }
    ProductID prodID{info.branchName()};
    op->setIncomingProductID(prodID);
    branchInfos_.emplace(std::move(prodID), std::move(info));
    // Check dictionaries for input product, not output product: let
    // output modules take care of that.
    checker.checkDictionaries(iType.className());
  }
  checker.reportMissingDictionaries();
}

art::EventAuxiliarySequence
art::RootIOPolicy::generateEventAuxiliarySequence(
  EntryNumberSequence const& enseq)
{
  EventAuxiliarySequence result;
  result.reserve(enseq.size());
  InputSourceMutexSentry sentry;
  auto const eventTree = currentDataTrees_[InEvent];
  auto auxBranch =
    eventTree->GetBranch(BranchTypeToAuxiliaryBranchName(InEvent).c_str());
  auto aux = std::make_unique<EventAuxiliary>();
  auto pAux = aux.get();
  auxBranch->SetAddress(&pAux);
  for (auto const entry : enseq) {
    auto err = eventTree->LoadTree(entry);
    if (err == -2) {
      // FIXME: Throw an error here, taking care to disconnect the
      // branch from the i/o buffer.
      // FIXME: -2 means entry number too big.
    }
    // Note: Root will overwrite the old event auxiliary with the new
    //       one.
    input::getEntry(auxBranch, entry);
    // Note: We are intentionally making a copy here of the fetched
    //       event auxiliary!
    result.push_back(*pAux);
  }
  // Disconnect the branch from the i/o buffer.
  auxBranch->SetAddress(nullptr);
  return result;
}

art::SpecProdList
art::RootIOPolicy::readFromFile(MixOpBase const& op,
                                EntryNumberSequence const& seq)
{
  SpecProdList result;
  result.reserve(seq.size());
  auto info = branchInfos_.find(op.incomingProductID());
  if (info == cend(branchInfos_)) {
    throw Exception(errors::LogicError)
      << "Branch corresponding to ProductID " << op.incomingProductID()
      << " has not initialized for reading.\n";
  }
  // Make sure the schema evolution is ready for ProductID
  if (op.branchType() == art::InEvent) {
    configureProductIDStreamer(branchIDListsInCurrentFile_.get());
  } else {
    configureProductIDStreamer(nullptr);
  }
  // Make sure we don't have a ProductGetter set.
  configureRefCoreStreamer();
  // Assume the sequence is ordered per
  // MixHelper::generateEventSequence.
  auto const b = seq.cbegin(), e = seq.cend();
  for (auto i = b; i != e; ++i) {
    auto fit = std::find(b, i, *i);
    if (fit == i) {
      // Need new product.
      auto ep = op.newIncomingWrappedProduct();
      result.emplace_back(ep); // ep now owned by shared_ptr
      // MT-FIXME
      InputSourceMutexSentry sentry;
      info->second.branch()->SetAddress(&ep);
      input::getEntry(info->second.branch(), *i);
    } else {
      // Already have one: find and use.
      auto pit = result.cbegin();
      std::advance(pit, std::distance(b, fit));
      result.emplace_back(*pit);
    }
  }
  return result;
}
