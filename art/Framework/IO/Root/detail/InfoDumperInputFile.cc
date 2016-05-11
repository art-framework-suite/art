#include "art/Framework/IO/Root/detail/InfoDumperInputFile.h"
#include "art/Framework/IO/Root/detail/setFileIndexPointer.h"
#include "art/Persistency/RootDB/SQLite3Wrapper.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Utilities/Exception.h"

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

  auto getFileIndex(TFile* file)
  {
    auto metaDataTree = reinterpret_cast<TTree*>(file->Get(art::rootNames::metaDataTreeName().data()));

    auto fileIndexUniquePtr = std::make_unique<art::FileIndex>();
    auto findexPtr = &*fileIndexUniquePtr;
    art::detail::setFileIndexPointer(file, metaDataTree, findexPtr);
    return *fileIndexUniquePtr;
  }

  using EntryNumbers = art::detail::InfoDumperInputFile::EntryNumbers;

  EntryNumbers
  getEntryNumbers(art::FileIndex::const_iterator& it,
                  art::FileIndex::const_iterator const end)
  {
    EntryNumbers entries;
    if (it == end)
      return {};

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
{}

void
art::detail::InfoDumperInputFile::print_file_index(std::ostream& os) const
{
  os << fileIndex_;
}

void
art::detail::InfoDumperInputFile::print_range_sets(std::ostream& os) const
{
  TTree* tree = static_cast<TTree*>(file_->Get(BranchTypeToProductTreeName(InRun).c_str()));
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
