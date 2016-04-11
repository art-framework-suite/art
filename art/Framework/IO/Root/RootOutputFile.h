#ifndef art_Framework_IO_Root_RootOutputFile_h
#define art_Framework_IO_Root_RootOutputFile_h
// vim: set sw=2:

// FIXME! There is an incestuous relationship between RootOutputFile and
// RootOutput that only works because the methods of RootOutput and
// OutputItem used by RootOutputFile are all inline. A correct and
// robust implementation would have a OutputItem defined in a separate
// file and the information (basket size, etc) in a different class in
// the main art/Framework/Root library accessed by both RootOutputFile
// and RootOutput. This has been entered as issue #2885.

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/IO/FileStatsCollector.h"
#include "art/Framework/IO/Root/DropMetaData.h"
#include "art/Framework/IO/Root/RootOutputClosingCriteria.h"
#include "art/Framework/IO/Root/RootOutputTree.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchID.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "canvas/Persistency/Provenance/ParameterSetBlob.h"
#include "canvas/Persistency/Provenance/ParameterSetMap.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/Selections.h"
#include "art/Persistency/RootDB/SQLite3Wrapper.h"
#include "boost/filesystem.hpp"

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "TROOT.h"

class TFile;
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
}

class art::RootOutputFile {
public: // TYPES

  enum class ClosureRequestMode { MaxEvents, MaxSize, Unset };
  using  RootOutputTreePtrArray = std::array<std::unique_ptr<RootOutputTree>, NumBranchTypes>;

  struct OutputItem {

    BranchDescription const* branchDescription_ {nullptr};
    mutable void const* product_ {nullptr};

    class Sorter {
    public:

      explicit Sorter(TTree* tree);
      bool  operator()(OutputItem const& lh, OutputItem const& rh) const;

    private:

      // Maps branch name to branch list index.
      std::map<std::string, int> treeMap_;
    };

    ~OutputItem() = default;

    explicit OutputItem(BranchDescription const* bd)
      : branchDescription_{bd}
    {}

    BranchID branchID() const
    {
      return branchDescription_->branchID();
    }

    std::string const& branchName() const
    {
      return branchDescription_->branchName();
    }

    bool operator<(OutputItem const& rh) const
    {
      return *branchDescription_ < *rh.branchDescription_;
    }

  };

  using OutputItemList = std::vector<OutputItem>;

  using OutputItemListArray =  std::array<OutputItemList, NumBranchTypes>;

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
                          bool fastCloning);

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
  void writeBranchIDListRegistry();
  void writeProductDependencies();
  void writeFileCatalogMetadata(FileStatsCollector const& stats,
                                FileCatalogMetadata::collection_type const&,
                                FileCatalogMetadata::collection_type const&);
  void writeResults(ResultsPrincipal & resp);
  void setAuxiliaryRangeSetID(RunPrincipal&);
  void setAuxiliaryRangeSetID(SubRunPrincipal&);
  void finishEndFile();
  void beginInputFile(FileBlock const&, bool fastClone);
  void respondToCloseInputFile(FileBlock const&);
  bool requestsToCloseFile() const;

  void selectProducts(FileBlock const&);

  std::string const& currentFileName() const { return file_; }

  bool maxEventsPerFileReached(FileIndex::EntryNumber_t const maxEventsPerFile) const;
  bool maxSizeReached(unsigned const maxFileSize) const;

private: // MEMBER FUNCTIONS

  void initializeFileContributors();
  void fillBranches(BranchType const&,
                    Principal const&,
                    std::vector<ProductProvenance>*);
  void insertAncestors(ProductProvenance const&,
                       Principal const&,
                       std::set<ProductProvenance>&);

private: // MEMBER DATA

  OutputModule const* om_;
  std::string file_;
  ClosingCriteria fileSwitchCriteria_;
  int const compressionLevel_;
  int64_t const saveMemoryObjectThreshold_;
  int64_t const treeMaxVirtualSize_;
  int const splitLevel_;
  int const basketSize_;
  DropMetaData dropMetaData_;
  bool dropMetaDataForDroppedData_;
  bool fastCloning_;
  bool currentlyFastCloning_ {true};
  std::shared_ptr<TFile> filePtr_;
  FileIndex fileIndex_ {};
  FileIndex::EntryNumber_t eventEntryNumber_ {0LL};
  FileIndex::EntryNumber_t subRunEntryNumber_ {0LL};
  FileIndex::EntryNumber_t runEntryNumber_ {0LL};
  TTree* metaDataTree_ {nullptr};
  TTree* fileIndexTree_ {nullptr};
  TTree* parentageTree_ {nullptr};
  TTree* eventHistoryTree_ {nullptr};
  EventAuxiliary const* pEventAux_ {nullptr};
  SubRunAuxiliary const* pSubRunAux_ {nullptr};
  RunAuxiliary const* pRunAux_ {nullptr};
  ResultsAuxiliary const *pResultsAux_ {nullptr};
  ProductProvenances eventProductProvenanceVector_ {};
  ProductProvenances subRunProductProvenanceVector_ {};
  ProductProvenances runProductProvenanceVector_ {};
  ProductProvenances resultsProductProvenanceVector_ {};
  ProductProvenances* pEventProductProvenanceVector_ {&eventProductProvenanceVector_};
  ProductProvenances* pSubRunProductProvenanceVector_ {&subRunProductProvenanceVector_};
  ProductProvenances* pRunProductProvenanceVector_ {&runProductProvenanceVector_};
  ProductProvenances* pResultsProductProvenanceVector_ {&resultsProductProvenanceVector_};
  History const* pHistory_ {nullptr};
  RootOutputTreePtrArray treePointers_;
  bool dataTypeReported_ {false};
  std::set<BranchID> branchesWithStoredHistory_ {};
  SQLite3Wrapper rootFileDB_;
  OutputItemListArray selectedOutputItemList_ {{}}; // filled by aggregation
  unsigned subRunRSID_ {-1u};
  unsigned runRSID_ {-1u};

};

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootOutputFile_h */
