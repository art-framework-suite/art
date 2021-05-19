// vim: set sw=2 expandtab :

#include "art/Framework/Services/FileServiceInterfaces/FileTransfer.h"
#include "art/Framework/Services/FileServiceInterfaces/FileTransferStatus.h"
#include "art/Framework/Services/Registry/ServiceDeclarationMacros.h"
#include "art/Framework/Services/Registry/ServiceDefinitionMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "fhiclcpp/fwd.h"

#include <fstream>
#include <string>

using namespace std;

using fhicl::ParameterSet;

namespace art {

  namespace {
    string const fileURI{"file://"};
  } // unnamed namespace

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

  TrivialFileTransfer::~TrivialFileTransfer() = default;

  TrivialFileTransfer::TrivialFileTransfer(
    TrivialFileTransfer::Parameters const&)
  {}

  TrivialFileTransfer::TrivialFileTransfer(TrivialFileTransfer const&) =
    default;

  TrivialFileTransfer::TrivialFileTransfer(TrivialFileTransfer&&) = default;

  TrivialFileTransfer& TrivialFileTransfer::operator=(
    TrivialFileTransfer const&) = default;

  TrivialFileTransfer& TrivialFileTransfer::operator=(TrivialFileTransfer&&) =
    default;

  int
  TrivialFileTransfer::doTranslateToLocalFilename(string const& uri,
                                                  string& fileFQname)
  {
    if (uri.substr(0, 7) != fileURI) {
      // Unexpected protocol: pass through.
      fileFQname = uri;
      return FileTransferStatus::SUCCESS;
    }
    FileTransferStatus stat = FileTransferStatus::PENDING;
    fileFQname = "";
    if (uri.substr(0, 7) != fileURI) {
      stat = FileTransferStatus::BAD_REQUEST;
      return stat;
    }
    string inFileName{uri.substr(7)};
    ifstream infile{inFileName};
    if (!infile) {
      stat = FileTransferStatus::NOT_FOUND;
      return stat;
    }
    fileFQname = inFileName;
    stat = FileTransferStatus::SUCCESS;
    return stat;
    // Implementation plan details -- alternatives not chosen:
    // x We could merely return the file name (the URI with file:// stripped
    // off).
    //   Since the SAM developers may look at this file as a template for their
    //   real GeneralFileTransfer service, it is perhaps better to do the work
    //   of making a copy into a designated area.
    // x We  merely strip the file:// from the URI; this adhoc class is not
    // beefed
    //   up to deal with genuine web access.
    // x An alternative would be to embed the last part of the file FQname into
    // the
    //   scratch file name, to try to maintain traceability in case things
    //   break.
  }

} // namespace art

DECLARE_ART_SERVICE_INTERFACE_IMPL(art::TrivialFileTransfer,
                                   art::FileTransfer,
                                   SHARED)

DEFINE_ART_SERVICE_INTERFACE_IMPL(art::TrivialFileTransfer, art::FileTransfer)
