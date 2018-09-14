#ifndef art_Framework_Services_Optional_TrivialFileTransfer_h
#define art_Framework_Services_Optional_TrivialFileTransfer_h
// vim: set sw=2 expandtab :

// ==========================================================================
// TrivialFileTransfer
//
// Class for service that return a fully qualified name of a file that
// has been copied into local scratch, when given a URI specifying a
// desired file.  This inherits from the art::FileTransfer base class.
// Eventually, GeneralFileTransfer will replace this class; this ad-hoc
// concrete class is meant as an early-testing scaffold.
// ==========================================================================

#include "art/Framework/Services/FileServiceInterfaces/FileTransfer.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "fhiclcpp/ParameterSet.h"

#include <iosfwd>
#include <string>

namespace art {

  class TrivialFileTransfer : public FileTransfer {
    // Configuration
  public:
    struct Config {};
    using Parameters = ServiceTable<Config>;
    // Special Member Functions
  public:
    ~TrivialFileTransfer();
    TrivialFileTransfer(Parameters const& pset);
    TrivialFileTransfer(TrivialFileTransfer const&);
    TrivialFileTransfer(TrivialFileTransfer&&);
    TrivialFileTransfer& operator=(TrivialFileTransfer const&);
    TrivialFileTransfer& operator=(TrivialFileTransfer&&);
    // Implementation -- Required by base class
  private:
    int doTranslateToLocalFilename(std::string const& uri,
                                   std::string& fileFQname) override;
  };

} // namespace art

DECLARE_ART_SERVICE_INTERFACE_IMPL(art::TrivialFileTransfer,
                                   art::FileTransfer,
                                   LEGACY)

#endif /* art_Framework_Services_Optional_TrivialFileTransfer_h */

// Local Variables:
// mode: c++
// End:
