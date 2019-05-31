#ifndef art_Framework_Services_Optional_TrivialFileDelivery_h
#define art_Framework_Services_Optional_TrivialFileDelivery_h
// vim: set sw=2 expandtab :

// ===========================================================================
// TrivialFileDelivery
//
// Class for service that returns a URI -- in this ad-hoc case, always
// just a file:// URI to an existing file -- when asked for a next
// input file.  (The list of files to "deliver" is specified at c'tor
// time, from input source parameters in the configuration file --
// when SAM has a real concrete FileDelivery class, it will be
// figuring out file URIs from the dataset requested.)  This inherits
// from the art::CatalogInterface base class.  Eventually, SAMProtocol
// will replace this class; this ad-hoc concrete class is meant as an
// early-testing scaffold.
// ===========================================================================

#include "art/Framework/Services/FileServiceInterfaces/CatalogInterface.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/RecursiveMutex.h"

#include <string>
#include <vector>

namespace art {

  class TrivialFileDelivery : public CatalogInterface {
    // Configuration
  public:
    struct Config {};
    using Parameters = ServiceTable<Config>;
    // Special Member Functions
  public:
    TrivialFileDelivery(Parameters const& config);
    // Implementation -- Required by base class
  private:
    void doConfigure(std::vector<std::string> const& items) override;
    int doGetNextFileURI(std::string& uri, double& waitTime) override;
    void doUpdateStatus(std::string const& uri,
                        FileDisposition status) override;
    void doOutputFileOpened(std::string const& module_label) override;
    void doOutputModuleInitiated(std::string const& module_label,
                                 fhicl::ParameterSet const& pset) override;
    void doOutputFileClosed(std::string const& module_label,
                            std::string const& file) override;
    void doEventSelected(std::string const& module_label,
                         EventID const& event_id,
                         HLTGlobalStatus const& acceptance_info) override;
    bool doIsSearchable() override;
    void doRewind() override;
    // Implementation details
  private:
    std::vector<std::string> extractFileListFromPset(
      fhicl::ParameterSet const& pset);
    std::string prependFileDesignation(std::string const& name) const;
    // Member data
  private:
    // Protects all data members.
    mutable hep::concurrency::RecursiveMutex mutex_{
      "art::TrivialFileDelivery::mutex_"};
    std::vector<std::string> fileList_{};
    std::vector<std::string>::const_iterator nextFile_{fileList_.cbegin()};
    std::vector<std::string>::const_iterator endOfFiles_{fileList_.cend()};
  };

} // namespace art

DECLARE_ART_SERVICE_INTERFACE_IMPL(art::TrivialFileDelivery,
                                   art::CatalogInterface,
                                   SHARED)

#endif /* art_Framework_Services_Optional_TrivialFileDelivery_h */

// Local Variables:
// mode: c++
// End:
