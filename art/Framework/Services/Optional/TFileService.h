#ifndef art_Framework_Services_Optional_TFileService_h
#define art_Framework_Services_Optional_TFileService_h

// ======================================================================
//
// TFileService
//
// ======================================================================

#include "art/Framework/IO/FileStatsCollector.h"
#include "art/Framework/Services/Optional/TFileDirectory.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Key.h"

namespace art {
  class ActivityRegistry;   // declaration only
  class ModuleDescription;  // declaration only
  class TFileService;       // defined below
}
namespace fhicl {
  class ParameterSet;       // declaration only
}

// ----------------------------------------------------------------------

class art::TFileService
  : public TFileDirectory
{
  // non-copyable:
  TFileService( TFileService const & ) = delete;
  TFileService operator = ( TFileService const & ) = delete;

public:

  static constexpr const char* default_tmpDir = "<filename>/TFileService";

  struct Config {
    fhicl::Atom<bool> closeFileFast   { fhicl::Key("closeFileFast"), false };
    fhicl::Atom<std::string> fileName { fhicl::Key("fileName") };
    fhicl::Atom<std::string> tmpDir   { fhicl::Key("tmpDir"), default_tmpDir };
  };

  // c'tor:
  using Parameters = ServiceTable<Config>;
  TFileService( ServiceTable<Config> const & config,
                art::ActivityRegistry      & r);

  // d'tor:
  ~TFileService();

  // accessor:
  TFile & file() const { return * file_; }

private:
  bool const closeFileFast_;
  FileStatsCollector fstats_;
  std::string filePattern_;
  std::string uniqueFilename_;

  // set current directory according to module name and prepare to create directory
  void setDirectoryName( art::ModuleDescription const & desc );
};  // TFileService

// ======================================================================

DECLARE_ART_SERVICE(TFileService, LEGACY)
#endif /* art_Framework_Services_Optional_TFileService_h */

// Local Variables:
// mode: c++
// End:
