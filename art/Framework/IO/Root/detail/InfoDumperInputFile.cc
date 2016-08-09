#include "art/Framework/IO/Root/detail/InfoDumperInputFile.h"
#include "art/Framework/IO/Root/detail/setFileIndexPointer.h"
#include "art/Framework/Principal/detail/orderedProcessNames.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/RootDB/SQLite3Wrapper.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Utilities/Exception.h"

#include <iomanip>

namespace {

  auto openFile(std::string const& fn)
  {
    std::unique_ptr<TFile> file {TFile::Open(fn.c_str(), "READ")};
    if (!file || file->IsZombie()) {
      throw art::Exception{art::errors::FileReadError}
      << "Unable to open file '" << fn << "' for reading.";
    }

    auto* key_ptr = file->GetKey("RootFileDB");
    if (key_ptr == nullptr) {
      throw art::Exception{art::errors::FileReadError}
      << "\nRequested DB, \"RootFileDB\" of type, \"tkeyvfs\", not present in file: \"" << fn << "\"\n"
      << "Either this is not an art/ROOT file, it is a corrupt art/ROOT file,\n"
      << "or it is an art/ROOT file produced with a version older than v1_00_12.\n";
    }
    return std::move(file);
  }

  auto getMetaData(TFile* file)
  {
    return static_cast<TTree*>(file->Get(art::rootNames::metaDataTreeName().data()));
  }

  auto getFileIndex(TFile* file)
  {
    auto md = getMetaData(file);
    auto fileIndexUniquePtr = std::make_unique<art::FileIndex>();
    auto findexPtr = &*fileIndexUniquePtr;
    art::detail::setFileIndexPointer(file, md, findexPtr);
    art::input::getEntry(md, 0);
    return *fileIndexUniquePtr;
  }

  auto getFileFormatVersion(TFile* file)
  {
    auto md = getMetaData(file);
    art::FileFormatVersion fftVersion {};
    auto fftPtr = &fftVersion;
    md->SetBranchAddress(art::rootNames::metaBranchRootName<art::FileFormatVersion>(), &fftPtr);
    art::input::getEntry(md, 0);
    return *fftPtr;
  }

  using EntryNumbers = art::detail::InfoDumperInputFile::EntryNumbers;

  EntryNumbers
  getEntryNumbers(art::FileIndex::const_iterator& it,
                  art::FileIndex::const_iterator const end)
  {
    if (it == end)
      return {};

    EntryNumbers entries;
    auto const eid = it->eventID_;
    for (; it != end && eid == it->eventID_; ++it) {
      entries.push_back(it->entry_);
    }
    return entries;
  }
}

art::detail::InfoDumperInputFile::InfoDumperInputFile(std::string const& filename)
  : file_{openFile(filename)}
  , fileIndex_{getFileIndex(file_.get())}
  , fileFormatVersion_{getFileFormatVersion(file_.get())}
{
  ProcessHistoryMap pHistMap;
  auto pHistMapPtr = &pHistMap;
  auto md = getMetaData(file_.get());
  md->SetBranchAddress(art::rootNames::metaBranchRootName<ProcessHistoryMap>(), &pHistMapPtr);
  art::input::getEntry(md, 0);
  ProcessHistoryRegistry::put(pHistMap);
}

void
art::detail::InfoDumperInputFile::print_event_list(std::ostream& os) const
{
  fileIndex_.print_event_list(os);
}

void
art::detail::InfoDumperInputFile::print_file_index(std::ostream& os) const
{
  os << fileIndex_;
}

void
art::detail::InfoDumperInputFile::print_process_history(std::ostream& os) const
{
  os << "\n Chronological list of process names for processes that\n"
     << " produced this file.\n\n";
  unsigned i {1u};
  for (auto const& process : orderedProcessNames())
    os << std::setw(5) << std::right << i++ << ". " << process << '\n';
}

void
art::detail::InfoDumperInputFile::print_range_sets(std::ostream& os) const
{
  if (fileFormatVersion_.value_ < 9) {
    std::ostringstream oss;
    oss << "Range-set information is not available for art/ROOT files with a format\n"
        << "version of \"" << fileFormatVersion_ << "\".\n";
    throw Exception{errors::FileReadError, "InfoDumperInputFile::print_range_sets"}
    << oss.str();
  }

  auto* tree = static_cast<TTree*>(file_->Get(BranchTypeToProductTreeName(InRun).c_str()));
  SQLite3Wrapper db {file_.get(), "RootFileDB"};

  auto it = fileIndex_.cbegin();
  auto const cend = fileIndex_.cend();

  while (it != cend) {
    if (it->getEntryType() != art::FileIndex::kRun) {
      ++it;
      continue;
    }
    auto const& entries = getEntryNumbers(it, cend);
    auto const& rs = getRangeSet(tree, entries, db, file_->GetName());
    os << rs << '\n';
  }
}

art::RunAuxiliary
art::detail::InfoDumperInputFile::getAuxiliary(TTree* tree, EntryNumber const entry) const
{
  auto aux  = std::make_unique<RunAuxiliary>();
  auto pAux = aux.get();
  TBranch* auxBranch = tree->GetBranch(BranchTypeToAuxiliaryBranchName(InRun).c_str());
  auxBranch->SetAddress(&pAux);
  tree->LoadTree(entry);
  auxBranch->GetEntry(entry);
  return *aux;
}

art::RangeSet
art::detail::InfoDumperInputFile::getRangeSet(TTree* tree,
                                              EntryNumbers const& entries,
                                              sqlite3* db,
                                              std::string const& filename) const
{
  auto auxResult = getAuxiliary(tree, entries[0]);
  auto rangeSet = detail::resolveRangeSet(db,
                                          filename,
                                          InRun,
                                          auxResult.rangeSetID());
  for(auto i = entries.cbegin()+1, e = entries.cend(); i!=e; ++i) {
    auto const& tmpAux = getAuxiliary(tree, *i);
    auxResult.mergeAuxiliary(tmpAux);
    auto const& tmpRangeSet = detail::resolveRangeSet(db,
                                                      filename,
                                                      InRun,
                                                      tmpAux.rangeSetID());
    rangeSet.merge(tmpRangeSet);
  }
  return rangeSet.collapse();
}
