#ifndef art_Framework_Services_Optional_TFileService_h
#define art_Framework_Services_Optional_TFileService_h

// ======================================================================
//
// TFileService
//
// ======================================================================

#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "art/Framework/Services/Optional/TFileDirectory.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

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
  // c'tor:
  TFileService( fhicl::ParameterSet const & cfg
              , art::ActivityRegistry     & r
              );

  // d'tor:
  ~TFileService();

  // accessor:
  TFile &
    file( ) const { return * file_; }

private:
  bool const closeFileFast_;
  PostCloseFileRenamer fileRenamer_;

  // set current directory according to module name and prepare to create directory
  void setDirectoryName( art::ModuleDescription const & desc );
};  // TFileService

// ======================================================================

DECLARE_ART_SERVICE(TFileService, LEGACY)
#endif /* art_Framework_Services_Optional_TFileService_h */

// Local Variables:
// mode: c++
// End:
