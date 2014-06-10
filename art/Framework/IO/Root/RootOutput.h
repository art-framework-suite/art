#ifndef art_Framework_IO_Root_RootOutput_h
#define art_Framework_IO_Root_RootOutput_h

// ======================================================================
//
// RootOutput
//
// ======================================================================

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
#include "boost/scoped_ptr.hpp"
#include "cpp0x/array"
#include "fhiclcpp/ParameterSet.h"
#include <string>

class TTree;

namespace art {

  class RootOutputFile;

  class RootOutput : public OutputModule {
    enum DropMetaData { DropNone, DropPrior, DropAll };
  public:
    friend class RootOutputFile;
    explicit RootOutput(fhicl::ParameterSet const& ps);
    virtual ~RootOutput();
private:
    std::string const & lastClosedFileName() const override;
    int const& compressionLevel() const {return compressionLevel_;}
    int const& basketSize() const {return basketSize_;}
    int const& splitLevel() const {return splitLevel_;}
    int64_t const& treeMaxVirtualSize() const {return treeMaxVirtualSize_;}
    int64_t const & saveMemoryObjectThreshold() const { return saveMemoryObjectThreshold_; }
    bool const& fastCloning() const {return fastCloning_;}
    DropMetaData const& dropMetaData() const {return dropMetaData_;}
    bool const& dropMetaDataForDroppedData() const {return dropMetaDataForDroppedData_;}

    struct OutputItem {
      class Sorter {
      public:
        explicit Sorter(TTree * tree);
        bool operator() (OutputItem const& lh, OutputItem const& rh) const;
      private:
        std::map<std::string, int> treeMap_;
      };

      OutputItem() : branchDescription_(0), product_(0) {}

      explicit OutputItem(BranchDescription const* bd) :
        branchDescription_(bd), product_(0) {}

      ~OutputItem() {}

      BranchID branchID() const { return branchDescription_->branchID(); }
      std::string const& branchName() const { return branchDescription_->branchName(); }

      BranchDescription const* branchDescription_;
      mutable void const* product_;

      bool operator <(OutputItem const& rh) const {
        return *branchDescription_ < *rh.branchDescription_;
      }
    };

    typedef std::vector<OutputItem> OutputItemList;

    typedef std::array<OutputItemList, NumBranchTypes> OutputItemListArray;

    OutputItemListArray const& selectedOutputItemList() const {return selectedOutputItemList_;}

    void openFile(FileBlock const& fb) override;
    void respondToOpenInputFile(FileBlock const& fb) override;
    void respondToCloseInputFile(FileBlock const& fb) override;
    void write(EventPrincipal const& e) override;
    void writeSubRun(SubRunPrincipal const& sr) override;
    void writeRun(RunPrincipal const& r) override;

    bool isFileOpen() const override;
    bool shouldWeCloseFile() const override;
    void doOpenFile();


    void startEndFile() override;
    void writeFileFormatVersion() override;
    void writeFileIndex() override;
    void writeEventHistory() override;
    void writeProcessConfigurationRegistry() override;
    void writeProcessHistoryRegistry() override;
    void writeParameterSetRegistry() override;
    void writeProductDescriptionRegistry() override;
    void writeParentageRegistry() override;
    void writeBranchIDListRegistry() override;
    void doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const & md) override;
    void writeProductDependencies() override;
    void finishEndFile() override;

    void fillSelectedItemList(BranchType branchtype, TTree *theTree);

    OutputItemListArray selectedOutputItemList_;
    std::string const catalog_;
    unsigned int const maxFileSize_;
    int const compressionLevel_;
    int const basketSize_;
    int const splitLevel_;
    int64_t const treeMaxVirtualSize_;
    int64_t const saveMemoryObjectThreshold_;
    bool fastCloning_;
    DropMetaData dropMetaData_;
    bool dropMetaDataForDroppedData_;
    std::string const moduleLabel_;
    int inputFileCount_;
    boost::scoped_ptr<RootOutputFile> rootOutputFile_;
    FileStatsCollector fstats_;
    std::string const filePattern_;
    std::string tmpDir_;
    std::string lastClosedFileName_;
  };

}  // art

// ======================================================================

#endif /* art_Framework_IO_Root_RootOutput_h */

// Local Variables:
// mode: c++
// End:
