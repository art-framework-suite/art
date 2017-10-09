#ifndef art_Framework_Services_Registry_ServiceRegistry_h
#define art_Framework_Services_Registry_ServiceRegistry_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Utilities/ScheduleID.h"
#include "cetlib/LibraryManager.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <vector>

namespace art {

  class ActivityRegistry;
  class EventProcessor;

  class ServiceRegistry {

    // Allow EventProcessor to set the manager.
    // friend class EventProcessor;

    template <typename T, art::ServiceScope>
    friend class ServiceHandle;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~ServiceRegistry();

  private: // MEMBER FUNCTIONS -- Special Member Functions
    ServiceRegistry();

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ServiceRegistry(ServiceRegistry const&) = delete;

    ServiceRegistry(ServiceRegistry&&) = delete;

    ServiceRegistry& operator=(ServiceRegistry const&) = delete;

    ServiceRegistry& operator=(ServiceRegistry&&) = delete;

  public: // MEMBER FUNCTIONS
    template <typename T>
    static bool
    isAvailable()
    {
      if (auto& mgr = instance().manager_) {
        return mgr->isAvailable<T>();
      }
      throw art::Exception(art::errors::ServiceNotFound, "Service")
        << " no ServiceRegistry has been set for this thread";
    }

    // FIXME: Cannot be private because of
    // art/test/Framework/Core/EventSelExc_t.cpp
    // FIXME: Cannot be private because of
    // art/test/Framework/Core/EventSelector_t.cpp
    // FIXME: Cannot be private because of
    // art/test/Framework/Core/EventSelWildcard_t.cpp
    static ServiceRegistry& instance();

    // FIXME: Cannot be private because of
    // art/test/Framework/Core/EventSelExc_t.cpp
    // FIXME: Cannot be private because of
    // art/test/Framework/Core/EventSelector_t.cpp
    // FIXME: Cannot be private because of
    // art/test/Framework/Core/EventSelWildcard_t.cpp
    void setManager(ServicesManager*);

  private:
    template <typename T>
    T&
    get() const
    {
      if (!manager_) {
        throw art::Exception(art::errors::ServiceNotFound, "Service")
          << " no ServiceRegistry has been set for this thread";
      }
      return manager_->get<T>();
    }

  private: // MEMBER DATA
    ServicesManager* manager_{nullptr};
  };

} // namespace art

#endif /* art_Framework_Services_Registry_ServiceRegistry_h */

// Local Variables:
// mode: c++
// End:
