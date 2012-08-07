#ifndef art_Framework_Services_UserInteraction_AdhocFileDelivery_h
#define art_Framework_Services_UserInteraction_AdhocFileDelivery_h

// -*- C++ -*-
//
// Package:     Services
// Class  :     AdhocFileDelivery
//
/*

 Description: Class for service that returns a URI -- in this ad hoc case, always
      just a file:// URI to an exisiting file -- when asked for a next input file.
      (The list of files to "deliver" is specified at ctor time, from input source
      parameters in the configuration file -- when SAM has a real concrete
      FileDelivery class, it will be figuring out file URI's from the dataset
      requested.)
      This inherits form the art::CatalogInterface base class.
      Eventually, SAMProtocol will replace this class; this adhoc concrete
      class is meant as an early-testing scaffold.

*/
//
// Original Author:  Mark Fischler
//         Created:  Mon  30 Jul, 2012
//

#include "art/Framework/Services/UserInteraction/CatalogInterface.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Common/HLTGlobalStatus.h"
#include "fhiclcpp/ParameterSet.h"
#include <string>

namespace art {
  class AdhocFileDelivery;
}

namespace art {
  typedef std::string URI;
  typedef std::string module_id_t;
}

namespace art {
  class FileDelivery;
}

namespace art {
  typedef std::string URI;
  typedef std::string module_id_t;
}

namespace art
{
class AdhocFileDelivery : public CatalogInterface
{
public:
  // ctor -- the services factory will expect this signature
  AdhocFileDelivery ( fhicl::ParameterSet const & ps,
        ActivityRegistry & acReg );

private:
  // Classes inheriting this interface must provide the following methods:
  int  doGetNextFileURI(URI & uri, double & waitTime);
  void doUpdateStatus (URI const & uri, int status);
  void doOutputFileOpened (module_id_t const & module_id);
  void doOutputModuleInitiated (module_id_t const & module_id,
                                        fhicl::ParameterSet const & pset);
  void doOutputFileClosed (module_id_t const & module_id,
                                   URI const & file);
  void doEventSelected(module_id_t const & module_id,
                     EventID event_id, HLTGlobalStatus acceptance_info);

  // helper functions
  std::vector<std::string> extractFileListFromPset
        (fhicl::ParameterSet const & pset);
  std::string prependFileDesignation(std::string const & name) const;

  // class data
  std::vector<std::string> fileList;
  std::vector<std::string>::const_iterator nextFile;
  std::vector<std::string>::const_iterator endOfFiles;
};
} // end of namespace art

#endif /* art_Framework_Services_UserInteraction_AdhocFileTransfer_h */

// Local Variables:
// mode: c++
// End:
