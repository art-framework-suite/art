#include "art/Framework/IO/Root/detail/InfoDumperInputFile.h"
#include "art/Framework/IO/Root/RootDB/SQLite3Wrapper.h"
#include "art/Framework/IO/Root/detail/rangeSetFromFileIndex.h"
#include "art/Framework/IO/Root/detail/readFileIndex.h"
#include "art/Framework/IO/Root/detail/readMetadata.h"
#include "art/Framework/IO/Root/detail/resolveRangeSet.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/orderedProcessNamesCollection.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/HorizontalRule.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace {

  auto
  openFile(std::string const& fn)
  {
    std::unique_ptr<TFile> file{TFile::Open(fn.c_str(), "READ")};
    if (!file || file->IsZombie()) {
      throw art::Exception{art::errors::FileReadError}
        << "Unable to open file '" << fn << "' for reading.";
    }

    auto* key_ptr = file->GetKey("RootFileDB");
    if (key_ptr == nullptr) {
      throw art::Exception{art::errors::FileReadError}
        << "Requested DB, \"RootFileDB\" of type, \"tkeyvfs\", not present in "
           "file: \""
        << fn << "\"\n"
        << "Either this is not an art/ROOT file, it is a corrupt art/ROOT "
           "file,\n"
        << "or it is an art/ROOT file produced with a version older than "
           "v1_00_12.\n";
    }
    return std::move(file);
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

art::detail::InfoDumperInputFile::InfoDumperInputFile(
  std::string const& filename)
  : file_{openFile(filename)}
{
  using namespace art::rootNames;
  std::unique_ptr<TTree> md{
    static_cast<TTree*>(file_->Get(metaDataTreeName().data()))};

  fileFormatVersion_ = detail::readMetadata<FileFormatVersion>(md.get());

  // Read BranchID lists if they exist
  detail::readMetadata(md.get(), branchIDLists_);

  // Read file index
  auto findexPtr = &fileIndex_;
  art::detail::readFileIndex(file_.get(), md.get(), findexPtr);

  // Read ProcessHistory
  pHistMap_ = detail::readMetadata<ProcessHistoryMap>(md.get());
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
  auto const& processNamesCollection = orderedProcessNamesCollection(pHistMap_);
  bool printHistoryLabel{false};
  if (processNamesCollection.empty()) {
    os << "\n No process history was recorded for this file.\n";
    return;
  } else if (processNamesCollection.size() > 1ull) {
    printHistoryLabel = true;
    os << "\n This file was produced with multiple processing histories.\n";
  }

  // Relying on a single character will be problematic if the
  // process-names collection has more than 26 elements.  Hopefully,
  // nobody will produce a job that involves the concatenation of more
  // than 26 inconsistent process histories.  If so, we'll solve the
  // process-history labeling below issue once we come to it.
  char hl{'A'};

  for (auto const& processNames : processNamesCollection) {
    unsigned i{1u};
    if (printHistoryLabel) {
      os << "\n Chronological list of process names for process history: "
         << hl++ << "\n\n";
    } else {
      os << "\n Chronological list of process names for processes that\n"
         << " produced this file.\n\n";
    }
    for (auto const& process : processNames) {
      os << ' ' << std::setw(4) << std::right << i++ << ". " << process << '\n';
    }
  }
}

void
art::detail::InfoDumperInputFile::print_branchIDLists(std::ostream& os) const
{
  if (fileFormatVersion_.value_ >= 10) {
    std::ostringstream oss;
    oss << "  BranchIDLists are not stored for art/ROOT files with a format\n"
        << "  version of \"" << fileFormatVersion_ << "\".\n";
    throw Exception{errors::FileReadError,
                    "InfoDumperInputFile::print_branchIDLists:\n"}
      << oss.str();
  }

  auto const& processNames = orderedProcessNamesCollection(pHistMap_);
  // For older file format versions, there cannot be more than one
  // ordered process name, due to the compatibility requirements that
  // were enforced in generating the file.
  assert(processNames.size() == 1ull);

  os << "\n List of BranchIDs produced for this file.  The BranchIDs are\n"
     << " grouped according to the process in which they were produced.  The\n"
     << " processes are presented in chronological order; however within each "
        "process,\n"
     << " the order of listed BranchIDs is not meaningful.\n";
  unsigned i{};
  for (auto const& process : processNames.front()) {
    os << "\n Process " << i + 1 << ": " << process << '\n';
    for (auto const& bid : branchIDLists_[i]) {
      os << "    " << bid << '\n';
    }
    ++i;
  }
}

void
art::detail::InfoDumperInputFile::print_range_sets(
  std::ostream& os,
  bool const compactRanges) const
{
  auto it = fileIndex_.cbegin();
  auto const cend = fileIndex_.cend();
  constexpr cet::HorizontalRule rule{30};
  std::string const rep{compactRanges ? "compact" : "full (default)"};

  if (fileFormatVersion_.value_ < 9) {
    os << '\n'
       << "*** This file has a format version of \"" << fileFormatVersion_
       << "\" and therefore\n"
       << "*** does not contain range-set information.  The printout below is\n"
       << "*** the range set art would assign to this file.\n\n"
       << "Representation: " << rep << '\n'
       << rule('-') << '\n';

    for (auto const& element : fileIndex_) {
      if (element.getEntryType() != art::FileIndex::kRun)
        continue;
      auto const& rs = rangeSetFromFileIndex(
        fileIndex_, element.eventID_.runID(), compactRanges);
      os << rs << '\n';
    }
    return;
  }

  auto* tree =
    static_cast<TTree*>(file_->Get(BranchTypeToProductTreeName(InRun).c_str()));
  SQLite3Wrapper db{file_.get(), "RootFileDB"};

  os << "Representation: " << rep << '\n' << rule('-') << '\n';
  while (it != cend) {
    if (it->getEntryType() != art::FileIndex::kRun) {
      ++it;
      continue;
    }
    // getEntryNumbers increments iterator!
    auto const& entries = getEntryNumbers(it, cend);
    auto const& rs =
      getRangeSet(tree, entries, db, file_->GetName(), compactRanges);
    os << rs << '\n';
  }
}

art::RunAuxiliary
art::detail::InfoDumperInputFile::getAuxiliary(TTree* tree,
                                               EntryNumber const entry) const
{
  auto aux = std::make_unique<RunAuxiliary>();
  auto pAux = aux.get();
  TBranch* auxBranch =
    tree->GetBranch(BranchTypeToAuxiliaryBranchName(InRun).c_str());
  auxBranch->SetAddress(&pAux);
  tree->LoadTree(entry);
  auxBranch->GetEntry(entry);
  return *aux;
}

art::RangeSet
art::detail::InfoDumperInputFile::getRangeSet(TTree* tree,
                                              EntryNumbers const& entries,
                                              sqlite3* db,
                                              std::string const& filename,
                                              bool const compactRanges) const
{
  auto resolve_info = [db, &filename](auto const id, bool const compact) {
    return detail::resolveRangeSetInfo(db, filename, InRun, id, compact);
  };

  auto auxResult = getAuxiliary(tree, entries[0]);
  auto rangeSetInfo = resolve_info(auxResult.rangeSetID(), compactRanges);

  for (auto i = entries.cbegin() + 1, e = entries.cend(); i != e; ++i) {
    auto const& tmpAux = getAuxiliary(tree, *i);
    rangeSetInfo.update(resolve_info(tmpAux.rangeSetID(), compactRanges),
                        compactRanges);
  }

  return resolveRangeSet(rangeSetInfo);
}
