#ifndef art_Framework_IO_Root_RootOutputFile_h
#define art_Framework_IO_Root_RootOutputFile_h
// vim: set sw=2:

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/IO/ClosingCriteria.h"
#include "art/Framework/IO/FileStatsCollector.h"
#include "art/Framework/IO/Root/DropMetaData.h"
#include "art/Framework/IO/Root/RootOutputTree.h"
#include "art/Framework/IO/Root/detail/DummyProductCache.h"
#include "art/Framework/Principal/RangeSetsSupported.h"
#include "art/Persistency/Provenance/Selections.h"
#include "boost/filesystem.hpp"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "canvas/Persistency/Provenance/ParameterSetBlob.h"
#include "canvas/Persistency/Provenance/ParameterSetMap.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "cetlib/sqlite/Connection.h"

#include <array>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "TFile.h"
#include "TROOT.h"

class TTree;

namespace art {
  class RootOutputFile;

  class ResultsPrincipal;
  class RootOutput;
  class History;
  class FileBlock;
  class EventAuxiliary;
  class SubRunAuxiliary;
  class RunAuxiliary;
  class ResultsAuxiliary;
  class RootFileBlock;
}

class art::RootOutputFile {
public: // TYPES
  enum class ClosureRequestMode { MaxEvents, MaxSize, Unset };
  using RootOutputTreePtrArray =
    std::array<std::unique_ptr<RootOutputTree>, NumBranchTypes>;

  struct OutputItem {

    BranchDescription const* branchDescription_{nullptr};
    mutable void const* product_{nullptr};

    explicit OutputItem(BranchDescription const* pd) : branchDescription_{pd} {}

    std::string const&
    branchName() const
    {
      return branchDescription_->branchName();
    }

    bool
    operator<(OutputItem const& rh) const
    {
      return *branchDescription_ < *rh.branchDescription_;
    }
  };

  using OutputItemList = std::set<OutputItem>;
  using OutputItemListArray = std::array<OutputItemList, NumBranchTypes>;

public: // MEMBER FUNCTIONS
  explicit RootOutputFile(OutputModule*,
                          std::string const& fileName,
                          ClosingCriteria const& fileSwitchCriteria,
                          int const compressionLevel,
                          int64_t const saveMemoryObjectThreshold,
                          int64_t const treeMaxVirtualSize,
                          int const splitLevel,
                          int const basketSize,
                          DropMetaData dropMetaData,
                          bool dropMetaDataForDroppedData,
                          bool fastCloningRequested,
                          bool const parentageEnabled,
                          bool const rangesEnabled,
                          bool const dbEnabled);

  void writeTTrees();

  void writeOne(EventPrincipal const&);
  void writeSubRun(SubRunPrincipal const&);
  void writeRun(RunPrincipal const&);
  void writeFileFormatVersion();
  void writeFileIndex();
  void writeEventHistory();
  void writeProcessConfigurationRegistry();
  void writeProcessHistoryRegistry();
  void writeParameterSetRegistry();
  void writeProductDescriptionRegistry();
  void writeParentageRegistry();
  void writeProductDependencies();
  void writeFileCatalogMetadata(FileStatsCollector const& stats,
                                FileCatalogMetadata::collection_type const&,
                                FileCatalogMetadata::collection_type const&);
  void writeResults(ResultsPrincipal& resp);
  void setRunAuxiliaryRangeSetID(RangeSet const&);
  void setSubRunAuxiliaryRangeSetID(RangeSet const&);
  void beginInputFile(RootFileBlock const*, bool fastClone);
  void incrementInputFileNumber();
  void respondToCloseInputFile(FileBlock const&);
  bool requestsToCloseFile();
  void
  setFileStatus(OutputFileStatus const ofs)
  {
    status_ = ofs;
  }

  void selectProducts();

  std::string const&
  currentFileName() const
  {
    return file_;
  }

  bool maxEventsPerFileReached(
    FileIndex::EntryNumber_t const maxEventsPerFile) const;
  bool maxSizeReached(unsigned const maxFileSize) const;

private: // MEMBER FUNCTIONS
  void writeRun_FileIndexPart(RunPrincipal const&);
  void writeSubRun_FileIndexPart(SubRunPrincipal const&);
  void writeOneEvent_EventPart(EventPrincipal const&);
  void writeOneEvent_HistoryPart(EventPrincipal const&);
  void writeOneEvent_FileIndexPart(EventPrincipal const&);

  void createDatabaseTables();

  template <BranchType>
  void fillBranches(Principal const&,
                    std::vector<ProductProvenance>*); // Defined in source.

  template <BranchType BT>
  std::enable_if_t<!detail::RangeSetsSupported<BT>::value,
                   art::EDProduct const*>
  getProduct(OutputHandle const&,
             RangeSet const& productRS,
             std::string const& wrappedName); // Defined in source.

  template <BranchType BT>
  std::enable_if_t<detail::RangeSetsSupported<BT>::value, art::EDProduct const*>
  getProduct(OutputHandle const&,
             RangeSet const& productRS,
             std::string const& wrappedName); // Defined in source.
  void insertParents(std::set<ProductProvenance>&,
                     Principal const&,
                     std::vector<ProductID> const&);

private: // MEMBER DATA
  OutputModule const* om_;
  std::string file_;
  ClosingCriteria fileSwitchCriteria_;
  OutputFileStatus status_{OutputFileStatus::Closed};
  int const compressionLevel_;
  int64_t const saveMemoryObjectThreshold_;
  int64_t const treeMaxVirtualSize_;
  int const splitLevel_;
  int const basketSize_;
  DropMetaData dropMetaData_;
  bool dropMetaDataForDroppedData_;
  bool fastCloningEnabledAtConstruction_;
  bool wasFastCloned_{false};
  std::unique_ptr<TFile> filePtr_; // File closed when d'tor called
  FileIndex fileIndex_{};
  FileProperties fp_{};
  TTree* metaDataTree_{nullptr};
  TTree* fileIndexTree_{nullptr};
  TTree* parentageTree_{nullptr};
  TTree* eventHistoryTree_{nullptr};
  EventAuxiliary const* pEventAux_{nullptr};
  SubRunAuxiliary const* pSubRunAux_{nullptr};
  RunAuxiliary const* pRunAux_{nullptr};
  ResultsAuxiliary const* pResultsAux_{nullptr};
  ProductProvenances eventProductProvenanceVector_{};
  ProductProvenances subRunProductProvenanceVector_{};
  ProductProvenances runProductProvenanceVector_{};
  ProductProvenances resultsProductProvenanceVector_{};
  ProductProvenances* pEventProductProvenanceVector_{
    &eventProductProvenanceVector_};
  ProductProvenances* pSubRunProductProvenanceVector_{
    &subRunProductProvenanceVector_};
  ProductProvenances* pRunProductProvenanceVector_{
    &runProductProvenanceVector_};
  ProductProvenances* pResultsProductProvenanceVector_{
    &resultsProductProvenanceVector_};
  History const* pHistory_{nullptr};
  RootOutputTreePtrArray treePointers_;
  bool dataTypeReported_{false};
  std::set<ProductID> branchesWithStoredHistory_{};
  cet::sqlite::Connection rootFileDB_; // Connection closed when d'tor
                                       // called.  DB written to file
                                       // when sqlite3_close is
                                       // called.
  OutputItemListArray selectedOutputItemList_{{}}; // filled by aggregation
  detail::DummyProductCache dummyProductCache_{};
  unsigned subRunRSID_{-1u};
  unsigned runRSID_{-1u};
  std::chrono::steady_clock::time_point beginTime_{
    std::chrono::steady_clock::now()};
  bool const parentageEnabled_{true};
  bool const rangesEnabled_{true};
  bool const dbEnabled_{true};
};

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootOutputFile_h */
