#ifndef art_Framework_Services_FileServiceInterfaces_CatalogInterface_h
#define art_Framework_Services_FileServiceInterfaces_CatalogInterface_h
// vim: set sw=2 expandtab :

// ======================================================================
// CatalogInterface
//
// Abstract base class for services that return URI's when asked to
// get next file in a series, and deal with declarations that output
// files have been written.  We have in mind that SAMProtocol will
// inherit from this interface class.
// ======================================================================

#include "art/Framework/Services/FileServiceInterfaces/FileDisposition.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Persistency/Common/fwd.h"
#include "cetlib/assert_only_one_thread.h"
#include "fhiclcpp/fwd.h"

#include <string>
#include <vector>

namespace art {
  class CatalogInterface;
  class EventID;
  class HLTGlobalStatus;

  class CatalogInterface {
  public:
    virtual ~CatalogInterface() noexcept = default;

  public:
    void configure(std::vector<std::string> const& items);
    int getNextFileURI(std::string& uri, double& waitTime);
    void updateStatus(std::string const& uri, FileDisposition status);
    void outputFileClosed(std::string const& module_label,
                          std::string const& fileFQname);
    void outputFileOpened(std::string const& module_label);
    void outputModuleInitiated(std::string const& module_label,
                               fhicl::ParameterSet const& pset);
    void eventSelected(std::string const& module_label,
                       EventID const& event_id,
                       HLTGlobalStatus const& acceptance_info);
    bool isSearchable();
    void rewind();

  private:
    virtual void doConfigure(std::vector<std::string> const& item) = 0;
    virtual int doGetNextFileURI(std::string& uri, double& waitTime) = 0;
    virtual void doUpdateStatus(std::string const& uri,
                                FileDisposition status) = 0;
    virtual void doOutputFileOpened(std::string const& module_label) = 0;
    virtual void doOutputModuleInitiated(std::string const& module_label,
                                         fhicl::ParameterSet const& pset) = 0;
    virtual void doOutputFileClosed(std::string const& module_label,
                                    std::string const& fileFQname) = 0;
    virtual void doEventSelected(std::string const& module_label,
                                 EventID const& event_id,
                                 HLTGlobalStatus const& acceptance_info) = 0;
    virtual bool doIsSearchable() = 0;
    virtual void doRewind() = 0;
  };

  inline void
  CatalogInterface::configure(std::vector<std::string> const& items)
  {
    CET_ASSERT_ONLY_ONE_THREAD();
    doConfigure(items);
  }

  inline int
  CatalogInterface::getNextFileURI(std::string& uri, double& waitTime)
  {
    CET_ASSERT_ONLY_ONE_THREAD();
    return doGetNextFileURI(uri, waitTime);
  }

  inline void
  CatalogInterface::updateStatus(std::string const& uri, FileDisposition status)
  {
    CET_ASSERT_ONLY_ONE_THREAD();
    doUpdateStatus(uri, status);
  }

  inline void
  CatalogInterface::outputFileClosed(std::string const& module_label,
                                     std::string const& fileFQname)
  {
    CET_ASSERT_ONLY_ONE_THREAD();
    doOutputFileClosed(module_label, fileFQname);
  }

  inline void
  CatalogInterface::outputFileOpened(std::string const& module_label)
  {
    CET_ASSERT_ONLY_ONE_THREAD();
    doOutputFileOpened(module_label);
  }

  inline void
  CatalogInterface::outputModuleInitiated(std::string const& module_label,
                                          fhicl::ParameterSet const& pset)
  {
    CET_ASSERT_ONLY_ONE_THREAD();
    doOutputModuleInitiated(module_label, pset);
  }

  inline void
  CatalogInterface::eventSelected(std::string const& module_label,
                                  EventID const& event_id,
                                  HLTGlobalStatus const& acceptance_info)
  {
    CET_ASSERT_ONLY_ONE_THREAD();
    doEventSelected(module_label, event_id, acceptance_info);
  }

  inline bool
  CatalogInterface::isSearchable()
  {
    CET_ASSERT_ONLY_ONE_THREAD();
    return doIsSearchable();
  }

  inline void
  CatalogInterface::rewind()
  {
    CET_ASSERT_ONLY_ONE_THREAD();
    doRewind();
  }

} // namespace art

DECLARE_ART_SERVICE_INTERFACE(art::CatalogInterface, GLOBAL)

#endif /* art_Framework_Services_FileServiceInterfaces_CatalogInterface_h */

// Local Variables:
// mode: c++
// End:
