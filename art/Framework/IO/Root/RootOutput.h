#ifndef art_Framework_IO_Root_RootOutput_h
#define art_Framework_IO_Root_RootOutput_h
// vim: set sw=2:

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/IO/FileStatsCollector.h"
#include "art/Framework/IO/Root/DropMetaData.h"
#include "boost/scoped_ptr.hpp"
#include "fhiclcpp/types/Atom.h"
#include <string>

class TTree;

namespace art {

class RootOutputFile;

class RootOutput : public OutputModule {

public: // MEMBER FUNCTIONS

  virtual ~RootOutput();

  static constexpr const char* default_tmpDir = "<some-tmp-dir>";

  struct Config {
    fhicl::Atom<std::string> catalog { fhicl::Name("catalog"), "" };
    fhicl::Atom<bool> dropAllEvents  { fhicl::Name("dropAllEvents"), false };
    fhicl::Atom<bool> dropAllSubRuns { fhicl::Name("dropAllSubRuns"), false };
    fhicl::Atom<std::string> fileName { fhicl::Name("fileName") };
    fhicl::Atom<std::string> tmpDir { fhicl::Name("tmpDir"), default_tmpDir };
    fhicl::Atom<int> maxSize { fhicl::Name("maxSize"), 0x7f000000 };
    fhicl::Atom<int> compressionLevel { fhicl::Name("compressionLevel"), 7 };
    fhicl::Atom<int64_t> saveMemoryObjectThreshold { fhicl::Name("saveMemoryObjectThreshold"), -1l };
    fhicl::Atom<int64_t> treeMaxVirtualSize { fhicl::Name("treeMaxVirtualSize"), -1 };
    fhicl::Atom<int> splitLevel { fhicl::Name("splitLevel"), 99 };
    fhicl::Atom<int> basketSize { fhicl::Name("basketSize"), 16384 };
    fhicl::Atom<bool> dropMetaDataForDroppedData { fhicl::Name("dropMetaDataForDroppedData"), false };
    fhicl::Atom<std::string> dropMetaData { fhicl::Name("dropMetaData"), "" };
  };

  using Parameters = OutputModule::Table<Config>;
  explicit RootOutput(Parameters const&);

  void
  postSelectProducts(FileBlock const&) override;

private: // MEMBER FUNCTIONS

  std::string const&
  lastClosedFileName() const override;

  void
  openFile(FileBlock const&) override;

  void
  respondToOpenInputFile(FileBlock const&) override;

  void
  respondToCloseInputFile(FileBlock const&) override;

  void
  write(EventPrincipal const&) override;

  void
  writeSubRun(SubRunPrincipal const&) override;

  void
  writeRun(RunPrincipal const&) override;

  bool
  isFileOpen() const override;

  bool
  shouldWeCloseFile() const override;

  void
  doOpenFile();

  void
  startEndFile() override;

  void
  writeFileFormatVersion() override;

  void
  writeFileIndex() override;

  void
  writeEventHistory() override;

  void
  writeProcessConfigurationRegistry() override;

  void
  writeProcessHistoryRegistry() override;

  void
  writeParameterSetRegistry() override;

  void
  writeProductDescriptionRegistry() override;

  void
  writeParentageRegistry() override;

  void
  writeBranchIDListRegistry() override;

  void
  doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const& md,
                             FileCatalogMetadata::collection_type const& ssmd)
                             override;

  void
  writeProductDependencies() override;

  void
  finishEndFile() override;

private:

  std::string const catalog_;
  bool dropAllEvents_;
  bool dropAllSubRuns_;
  std::string const moduleLabel_;
  int inputFileCount_;
  boost::scoped_ptr<RootOutputFile> rootOutputFile_;
  FileStatsCollector fstats_;
  std::string const filePattern_;
  std::string tmpDir_;
  std::string lastClosedFileName_;

  // We keep this set of data members for the use
  // of RootOutputFile.
  unsigned int const maxFileSize_;
  int const compressionLevel_;
  int64_t const saveMemoryObjectThreshold_;
  int64_t const treeMaxVirtualSize_;
  int const splitLevel_;
  int const basketSize_;
  DropMetaData dropMetaData_;
  bool dropMetaDataForDroppedData_;

  // We keep this for the use of RootOutputFile
  // and we also use it during file open to
  // make some choices.
  bool fastCloning_;
};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif // art_Framework_IO_Root_RootOutput_h
