#ifndef art_Framework_Services_FileServiceInterfaces_FileTransfer_h
#define art_Framework_Services_FileServiceInterfaces_FileTransfer_h

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
  class FileTransfer;
}

class art::FileTransfer {
public:
  virtual ~FileTransfer() noexcept = default;

  int translateToLocalFilename(std::string const& uri, std::string& fileFQname);

private:
  virtual int doTranslateToLocalFilename(std::string const& uri,
                                         std::string& fileFQname) = 0;
};

inline int
art::FileTransfer::translateToLocalFilename(std::string const& uri,
                                            std::string& fileFQname)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  return doTranslateToLocalFilename(uri, fileFQname);
}

DECLARE_ART_SERVICE_INTERFACE(art::FileTransfer, LEGACY)
#endif /* art_Framework_Services_FileServiceInterfaces_FileTransfer_h */

// Local Variables:
// mode: c++
// End:
