#ifndef art_Framework_Services_Optional_TrivialFileTransfer_h
#define art_Framework_Services_Optional_TrivialFileTransfer_h

// -*- C++ -*-
//
// Package:     Services
// Class  :     TrivialFileTransfer
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

    // ctor -- the services factory will expect this signature
    TrivialFileTransfer(Parameters const & pset, ActivityRegistry & acReg);

  private:
    // Classes inheriting FileTransfer interface must provide the following method:
    int doTranslateToLocalFilename(std::string const & uri, std::string & fileFQname) override;

    // helper functions
    int stripURI(std::string const & uri, std::string & inFileName) const;
    int copyFile(std::ifstream & in, std::ofstream & out) const;

    // class data
    std::string scratchArea;
  };
} // end of art namespace
DECLARE_ART_SERVICE_INTERFACE_IMPL(art::TrivialFileTransfer, art::FileTransfer, LEGACY)
#endif /* art_Framework_Services_Optional_TrivialFileTransfer_h */

// Local Variables:
// mode: c++
// End:
