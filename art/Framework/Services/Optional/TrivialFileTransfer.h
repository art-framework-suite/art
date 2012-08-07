#ifndef art_Framework_Services_UserInteraction_AdhocFileTransfer_h
#define art_Framework_Services_UserInteraction_AdhocFileTransfer_h

// -*- C++ -*-
//
// Package:     Services
// Class  :     AdhocFileTransfer
//
/*

 Description: Class for service that return a fully qualified file name
      of a file that has been copied into local scratch, when given a URI
      specifying a desired file.
      This inherits from the art::FileTransfer base class.
      Eventually, GeneralFileTransfer will freplace this class; this adhoc concrete
      class is meant as an early-testing scaffold.

*/
//
// Original Author:  Mark Fischler
//         Created:  Fri  27 Jul, 2012
//

#include "art/Framework/Services/UserInteraction/FileTransfer.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "fhiclcpp/ParameterSet.h"
#include <string>

namespace art {
  class AdhocFileTransfer;
}

namespace art {
class AdhocFileTransfer : public FileTransfer
{
public:
  // ctor -- the services factory will expect this signature
  AdhocFileTransfer ( fhicl::ParameterSet const & pset, ActivityRegistry & acReg );

private:
  // Classes inheriting FileTransfer interface must provide the following method:
  virtual int doCopyToScratch( art::URI const & uri, std::string & fileFQname );

  // helper functions
  int stripURI(art::URI const & uri, std::string & inFileName) const;
  int copyFile(std::ifstream & in, std::ofstream & out) const;

  // class data
  std::string scratchArea;
};
} // end of art namespace
#endif /* art_Framework_Services_UserInteraction_AdhocFileTransfer_h */

// Local Variables:
// mode: c++
// End:
