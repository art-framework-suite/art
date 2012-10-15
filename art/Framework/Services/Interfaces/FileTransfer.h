#ifndef art_Framework_Services_Interfaces_FileTransfer_h
#define art_Framework_Services_Interfaces_FileTransfer_h

// -*- C++ -*-
//
// Package:     Services
// Class  :     FileTransfer
//
/*

 Description: Abstract base class for services that return a fully qualified file name
      of a file that has been copied into local scratch, when given a URI specifying
      a desired file.   We have in mind that GeneralFileTransfer will inherit from
      this interface class.

*/
//
// Original Author:  Mark Fischler
//         Created:  Wed  25 Jul, 2012
//

#include "art/Framework/Services/Registry/ServiceMacros.h"

#include <string>

namespace art {
  class FileTransfer;
}

class art::FileTransfer {
public:
  int translateToLocalFilename(std::string const & uri,
                               std::string & fileFQname);

  // Remaining boilerplate:
  virtual ~FileTransfer() = default;

private:
  // Classes inheriting this interface must provide the following method:
  virtual
  int
  doTranslateToLocalFilename(std::string const & uri,
                             std::string & fileFQname) = 0;
};

inline
int
art::FileTransfer::
translateToLocalFilename(std::string const & uri,
                         std::string & fileFQname)
{
  return doTranslateToLocalFilename(uri, fileFQname);
}

DECLARE_ART_SERVICE_INTERFACE(art::FileTransfer,LEGACY)
#endif /* art_Framework_Services_Interfaces_FileTransfer_h */

// Local Variables:
// mode: c++
// End:
