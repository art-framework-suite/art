#ifndef art_Framework_Services_Optional_TrivialFileDelivery_h
#define art_Framework_Services_Optional_TrivialFileDelivery_h

// -*- C++ -*-
//
// Package:     Services
// Class  :     TrivialFileDelivery
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

#include "art/Framework/Services/FileServiceInterfaces/CatalogInterface.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "fhiclcpp/ParameterSet.h"
#include <string>

namespace art {
  class TrivialFileDelivery;
}

namespace art {
  class TrivialFileDelivery : public CatalogInterface {
  public:
    // configuration
    struct Config {};
    using Parameters = ServiceTable<Config>;

    // ctor -- the services factory will expect this signature
    TrivialFileDelivery(Parameters const & config,
                        ActivityRegistry & acReg);

  private:
    // Classes inheriting this interface must provide the following methods:
    void doConfigure(std::vector<std::string> const & items) override;
    int  doGetNextFileURI(std::string & uri, double & waitTime) override;
    void doUpdateStatus(std::string const & uri, FileDisposition status) override;
    void doOutputFileOpened(std::string const & module_label) override;
    void doOutputModuleInitiated(std::string const & module_label,
                                 fhicl::ParameterSet const & pset) override;
    void doOutputFileClosed(std::string const & module_label,
                            std::string const & file) override;
    void doEventSelected(std::string const & module_label,
                         EventID const & event_id,
                         HLTGlobalStatus const & acceptance_info) override;
    bool doIsSearchable() override;
    void doRewind() override;

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

DECLARE_ART_SERVICE_INTERFACE_IMPL(art::TrivialFileDelivery, art::CatalogInterface, LEGACY)
#endif /* art_Framework_Services_Optional_TrivialFileDelivery_h */

// Local Variables:
// mode: c++
// End:
