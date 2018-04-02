#ifndef art_Framework_Services_Optional_TFileService_h
#define art_Framework_Services_Optional_TFileService_h

// ======================================================================
//
// TFileService
//
// ======================================================================

#include "art/Framework/Core/OutputFileGranularity.h"
#include "art/Framework/IO/ClosingCriteria.h"
#include "art/Framework/IO/FileStatsCollector.h"
#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "art/Framework/Services/Optional/TFileDirectory.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"
#include "fhiclcpp/types/OptionalTable.h"

namespace art {
  class ActivityRegistry;  // declaration only
  class ModuleDescription; // declaration only
  class TFileService;      // defined below
} // namespace art

namespace fhicl {
  class ParameterSet; // declaration only
}

// ----------------------------------------------------------------------

class art::TFileService : public TFileDirectory {
  // non-copyable:
  TFileService(TFileService const&) = delete;
  TFileService operator=(TFileService const&) = delete;

public:
  static constexpr const char* default_tmpDir = "<parent-path-of-filename>";

  struct Config {
    fhicl::Atom<bool> closeFileFast{fhicl::Name("closeFileFast"), true};
    fhicl::Atom<std::string> fileName{fhicl::Name("fileName")};
    fhicl::Atom<std::string> tmpDir{fhicl::Name("tmpDir"), default_tmpDir};
    fhicl::OptionalTable<ClosingCriteria::Config> fileProperties{
      fhicl::Name("fileProperties")};
  };

  // c'tor:
  using Parameters = ServiceTable<Config>;
  TFileService(ServiceTable<Config> const& config, art::ActivityRegistry& r);

  // d'tor:
  ~TFileService();

  using Callback_t = TFileDirectory::Callback_t;

  void registerFileSwitchCallback(Callback_t c);

  template <typename T>
  void registerFileSwitchCallback(T* provider, void (T::*)());

  // accessor:
  TFile&
  file() const
  {
    return *file_;
  }

private:
  bool const closeFileFast_;
  FileStatsCollector fstats_;
  PostCloseFileRenamer fRenamer_{fstats_};
  std::string filePattern_;
  std::string uniqueFilename_;
  std::string tmpDir_;

  // File-switching mechanics
  std::string lastClosedFile_{};
  Granularity currentGranularity_{Granularity::Unset};
  std::chrono::steady_clock::time_point beginTime_{};
  // Do not use default-constructed "ClosingCriteria" as that will be
  // sure to trigger a file switch at the first call of
  // requestsToCloseFile_().
  ClosingCriteria fileSwitchCriteria_{ClosingCriteria::Config{}};
  OutputFileStatus status_{OutputFileStatus::Closed};
  FileProperties fp_{};

  // set current directory according to module name and prepare to create
  // directory
  void setDirectoryName_(art::ModuleDescription const& desc);
  void openFile_();
  void closeFile_();
  void maybeSwitchFiles_();
  bool requestsToCloseFile_();
  std::string fileNameAtOpen_();
  std::string fileNameAtClose_(std::string const&);
}; // TFileService

// ======================================================================

template <typename T>
void
art::TFileService::registerFileSwitchCallback(T* provider, void (T::*f)())
{
  auto cb = [provider, f] { return (provider->*f)(); };
  registerFileSwitchCallback(cb);
}

DECLARE_ART_SERVICE(TFileService, LEGACY)
#endif /* art_Framework_Services_Optional_TFileService_h */

// Local Variables:
// mode: c++
// End:
