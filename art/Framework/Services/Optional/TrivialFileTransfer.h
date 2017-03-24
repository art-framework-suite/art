#ifndef art_Framework_Services_Optional_TrivialFileTransfer_h
#define art_Framework_Services_Optional_TrivialFileTransfer_h

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
#include <string>

namespace art {
  class TrivialFileTransfer;
}

namespace art {
  class TrivialFileTransfer : public FileTransfer {
  public:
    // configuration
    struct Config{};
    using Parameters = ServiceTable<Config>;

    TrivialFileTransfer(Parameters const& pset);

  private:

    int doTranslateToLocalFilename(std::string const& uri, std::string& fileFQname) override;

    int stripURI(std::string const& uri, std::string& inFileName) const;
    int copyFile(std::ifstream& in, std::ofstream& out) const;
  };
} // end of art namespace

DECLARE_ART_SERVICE_INTERFACE_IMPL(art::TrivialFileTransfer, art::FileTransfer, LEGACY)
#endif /* art_Framework_Services_Optional_TrivialFileTransfer_h */

// Local Variables:
// mode: c++
// End:
