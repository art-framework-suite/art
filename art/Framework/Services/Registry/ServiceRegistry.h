#ifndef art_Framework_Services_Registry_ServiceRegistry_h
#define art_Framework_Services_Registry_ServiceRegistry_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Utilities/PluginSuffixes.h"
#include "cetlib/LibraryManager.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <vector>

namespace art {
  namespace test {
    void set_manager_for_tests(ServicesManager*);
  }

  class ActivityRegistry;
  class EventProcessor;

  class ServiceRegistry {

    // Allow EventProcessor to set the manager.  Also, allow a testing
    // function to set it.
    friend class EventProcessor;
    friend void art::test::set_manager_for_tests(ServicesManager*);

    template <typename T, art::ServiceScope>
    friend class ServiceHandle;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~ServiceRegistry() noexcept;
    ServiceRegistry(ServiceRegistry const&) = delete;
    ServiceRegistry(ServiceRegistry&&) = delete;
    ServiceRegistry& operator=(ServiceRegistry const&) = delete;
    ServiceRegistry& operator=(ServiceRegistry&&) = delete;

  private: // MEMBER FUNCTIONS -- Special Member Functions
    ServiceRegistry() noexcept;

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

  private:
    static ServiceRegistry& instance() noexcept;

    void setManager(ServicesManager*) noexcept;

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
    cet::exempt_ptr<ServicesManager> manager_{nullptr};
  };

} // namespace art

#endif /* art_Framework_Services_Registry_ServiceRegistry_h */

// Local Variables:
// mode: c++
// End:
