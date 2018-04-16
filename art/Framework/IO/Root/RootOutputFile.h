#ifndef art_Framework_IO_Root_RootOutputFile_h
#define art_Framework_IO_Root_RootOutputFile_h
// vim: set sw=2 expandtab :

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
#include "hep_concurrency/RecursiveMutex.h"

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
  class ResultsPrincipal;
  class RootOutput;
  class History;
  class FileBlock;
  class EventAuxiliary;
  class SubRunAuxiliary;
  class RunAuxiliary;
  class ResultsAuxiliary;
  class RootFileBlock;
  class RootOutputFile {
  public: // TYPES
    enum class ClosureRequestMode { MaxEvents = 0, MaxSize = 1, Unset = 2 };
    using RootOutputTreePtrArray =
      std::array<std::unique_ptr<RootOutputTree>, NumBranchTypes>;
    struct OutputItem {
    public: // MEMBER FUNCTIONS -- Special Member Functions
      ~OutputItem();
      explicit OutputItem(BranchDescription const& bd);

    public: // MEMBER FUNCTIONS
      std::string const& branchName() const;
      bool operator<(OutputItem const& rh) const;

    public: // MEMBER DATA
      BranchDescription const branchDescription_;
      mutable void const* product_;
    };

  public: // MEMBER FUNCTIONS -- Static API
    static bool shouldFastClone(bool const fastCloningSet,
                                bool const fastCloning,
                                bool const wantAllEvents,
                                ClosingCriteria const& cc);

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~RootOutputFile();
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
                            bool fastCloningRequested);
    RootOutputFile(RootOutputFile const&) = delete;
    RootOutputFile(RootOutputFile&&) = delete;
    RootOutputFile& operator=(RootOutputFile const&) = delete;
    RootOutputFile& operator=(RootOutputFile&&) = delete;

  public: // MEMBER FUNCTIONS
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
    void setFileStatus(OutputFileStatus const ofs);
    void selectProducts();
    std::string const& currentFileName() const;
    bool maxEventsPerFileReached(
      FileIndex::EntryNumber_t const maxEventsPerFile) const;
    bool maxSizeReached(unsigned const maxFileSize) const;

  private: // MEMBER FUNCTIONS
    template <BranchType>
    void fillBranches(Principal const&, std::vector<ProductProvenance>*);
    template <BranchType BT>
    std::enable_if_t<!detail::RangeSetsSupported<BT>::value, EDProduct const*>
    getProduct(OutputHandle const&,
               RangeSet const& productRS,
               std::string const& wrappedName);
    template <BranchType BT>
    std::enable_if_t<detail::RangeSetsSupported<BT>::value, EDProduct const*>
    getProduct(OutputHandle const&,
               RangeSet const& productRS,
               std::string const& wrappedName);

  private: // MEMBER DATA
    mutable hep::concurrency::RecursiveMutex mutex_;
    OutputModule const* om_;
    std::string file_;
    ClosingCriteria fileSwitchCriteria_;
    OutputFileStatus status_;
    int const compressionLevel_;
    int64_t const saveMemoryObjectThreshold_;
    int64_t const treeMaxVirtualSize_;
    int const splitLevel_;
    int const basketSize_;
    DropMetaData dropMetaData_;
    bool dropMetaDataForDroppedData_;
    bool fastCloningEnabledAtConstruction_;
    bool wasFastCloned_;
    std::unique_ptr<TFile> filePtr_;
    FileIndex fileIndex_;
    FileProperties fp_;
    TTree* metaDataTree_;
    TTree* fileIndexTree_;
    TTree* parentageTree_;
    TTree* eventHistoryTree_;
    EventAuxiliary const* pEventAux_;
    SubRunAuxiliary const* pSubRunAux_;
    RunAuxiliary const* pRunAux_;
    ResultsAuxiliary const* pResultsAux_;
    ProductProvenances eventProductProvenanceVector_;
    ProductProvenances subRunProductProvenanceVector_;
    ProductProvenances runProductProvenanceVector_;
    ProductProvenances resultsProductProvenanceVector_;
    ProductProvenances* pEventProductProvenanceVector_;
    ProductProvenances* pSubRunProductProvenanceVector_;
    ProductProvenances* pRunProductProvenanceVector_;
    ProductProvenances* pResultsProductProvenanceVector_;
    History const* pHistory_;
    RootOutputTreePtrArray treePointers_;
    bool dataTypeReported_;
    std::array<ProductDescriptionsByID, NumBranchTypes> descriptionsToPersist_;
    std::unique_ptr<cet::sqlite::Connection> rootFileDB_;
    std::array<std::set<OutputItem>, NumBranchTypes> selectedOutputItemList_;
    detail::DummyProductCache dummyProductCache_;
    unsigned subRunRSID_;
    unsigned runRSID_;
    std::chrono::steady_clock::time_point beginTime_;
  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootOutputFile_h */
