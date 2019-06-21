#ifndef art_Framework_Services_FileServiceInterfaces_FileTransfer_h
#define art_Framework_Services_FileServiceInterfaces_FileTransfer_h
// vim: set sw=2 expandtab :

// ====================================================================
// FileTransfer
//
// Abstract base class for services that return a fully qualified name
// of a file that has been copied into local scratch, when given a URI
// specifying a desired file.  We have in mind that
// GeneralFileTransfer will inherit from this interface class.
// ====================================================================

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "cetlib/assert_only_one_thread.h"

#include <string>

namespace art {

  class FileTransfer {
  public:
    virtual ~FileTransfer() noexcept = default;
    int translateToLocalFilename(std::string const& uri,
                                 std::string& fileFQname);

  private:
    virtual int doTranslateToLocalFilename(std::string const& uri,
                                           std::string& fileFQname) = 0;
  };

  inline int
  FileTransfer::translateToLocalFilename(std::string const& uri,
                                         std::string& fileFQname)
  {
    return doTranslateToLocalFilename(uri, fileFQname);
  }

} // namespace art

DECLARE_ART_SERVICE_INTERFACE(art::FileTransfer, SHARED)

#endif /* art_Framework_Services_FileServiceInterfaces_FileTransfer_h */

// Local Variables:
// mode: c++
// End:
