#ifndef art_Framework_Services_UserInteraction_FileTransfer_h
#define art_Framework_Services_UserInteraction_FileTransfer_h

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

#include <string>

namespace art {
  class FileTransfer;
}

namespace art {
  typedef std::string URI;
}

class art::FileTransfer
{
public:
  int copyToScratch( art::URI const & uri, std::string & fileFQname );

  // Remaining boilerplate:
  virtual ~FileTransfer() = 0;

private:
  // Classes inheriting this interface must provide the following method:
  virtual int doCopyToScratch( art::URI const & uri, std::string & fileFQname ) = 0;
};

inline int art::FileTransfer::copyToScratch( art::URI const & uri, 
					     std::string & fileFQname ) {
  return doCopyToScratch (uri, fileFQname);
}

#endif /* art_Framework_Services_UserInteraction_FileTransfer_h */

// Local Variables:
// mode: c++
// End:
