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

  using  RootOutputTreePtrArray = std::array<std::unique_ptr<RootOutputTree>, NumBranchTypes>;

  struct OutputItem {

    BranchDescription const* branchDescription_;
    mutable void const* product_;

    class Sorter {

  public:

      explicit
      Sorter(TTree* tree);

      bool
      operator()(OutputItem const& lh, OutputItem const& rh) const;

  private:

      // Maps branch name to branch list index.
      std::map<std::string, int> treeMap_;

    };

    ~OutputItem()
      {
      }

    OutputItem()
      : branchDescription_(0)
      , product_(0)
      {
      }

    explicit
    OutputItem(BranchDescription const* bd)
      : branchDescription_(bd)
      , product_(0)
      {
      }

    BranchID
    branchID() const
      {
        return branchDescription_->branchID();
      }

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

  using OutputItemList = std::vector<OutputItem>;

  using OutputItemListArray =  std::array<OutputItemList, NumBranchTypes>;

public: // MEMBER FUNCTIONS

  explicit RootOutputFile(OutputModule*, std::string const& fileName,
                          unsigned int const maxFileSize,
                          int const compressionLevel,
                          int64_t const saveMemoryObjectThreshold,
                          int64_t const treeMaxVirtualSize,
                          int const splitLevel, int const basketSize,
                          DropMetaData dropMetaData,
                          bool dropMetaDataForDroppedData,
                          bool fastCloning);
  // use compiler-generated copy c'tor, copy assignment, and d'tor
  void writeOne(EventPrincipal const&);
  //void endFile();
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
  void finishEndFile();
  void beginInputFile(FileBlock const&, bool fastClone);
  void respondToCloseInputFile(FileBlock const&);
  bool shouldWeCloseFile() const;

  void
  selectProducts(FileBlock const&);

  std::string const& currentFileName() const
  {
    return file_;
  }

private: // MEMBER FUNCTIONS

  void fillBranches(BranchType const&, Principal const&,
                    std::vector<ProductProvenance>*);
  void insertAncestors(ProductProvenance const&, Principal const&,
                       std::set<ProductProvenance>&);

private: // MEMBER DATA

  std::string file_;
  OutputModule const* om_;
  unsigned int const maxFileSize_;
  int const compressionLevel_;
  int64_t const saveMemoryObjectThreshold_;
  int64_t const treeMaxVirtualSize_;
  int const splitLevel_;
  int const basketSize_;
  DropMetaData dropMetaData_;
  bool dropMetaDataForDroppedData_;
  bool fastCloning_;
  bool currentlyFastCloning_;
  std::shared_ptr<TFile> filePtr_;
  FileIndex fileIndex_;
  FileIndex::EntryNumber_t eventEntryNumber_;
  FileIndex::EntryNumber_t subRunEntryNumber_;
  FileIndex::EntryNumber_t runEntryNumber_;
  TTree* metaDataTree_;
  TTree* fileIndexTree_;
  TTree* parentageTree_;
  TTree* eventHistoryTree_;
  EventAuxiliary const* pEventAux_;
  SubRunAuxiliary const* pSubRunAux_;
  RunAuxiliary const* pRunAux_;
  ResultsAuxiliary const *pResultsAux_;
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
  std::set<BranchID> branchesWithStoredHistory_;
  SQLite3Wrapper metaDataHandle_;
  OutputItemListArray selectedOutputItemList_;

};

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootOutputFile_h */
