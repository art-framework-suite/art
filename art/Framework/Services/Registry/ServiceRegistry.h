#ifndef art_Framework_Services_Registry_ServiceRegistry_h
#define art_Framework_Services_Registry_ServiceRegistry_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/exempt_ptr.h"

namespace art {

  class ServiceRegistry {
    // Allow EventProcessor to set the manager.  Also, allow a testing
    // function to set it.
    friend class ServicesManager;

    template <typename T, art::ServiceScope>
    friend class ServiceHandle;

  public:
    ~ServiceRegistry() noexcept;
    ServiceRegistry(ServiceRegistry const&) = delete;
    ServiceRegistry(ServiceRegistry&&) = delete;
    ServiceRegistry& operator=(ServiceRegistry const&) = delete;
    ServiceRegistry& operator=(ServiceRegistry&&) = delete;

    template <typename T>
    static bool
    isAvailable()
    {
      if (auto& mgr = instance().manager_) {
        return mgr->isAvailable<T>();
      }
      throw Exception(errors::ServiceNotFound, "Service")
        << " no ServiceRegistry has been set for this thread";
    }

  private:
    ServiceRegistry() noexcept;
    static ServiceRegistry& instance() noexcept;

    void setManager(ServicesManager*);

    template <typename T>
    T&
    get() const
    {
      if (!manager_) {
        throw Exception(errors::ServiceNotFound, "Service")
          << " no ServiceRegistry has been set for this thread";
      }
      return manager_->get<T>();
    }

    cet::exempt_ptr<ServicesManager> manager_{nullptr};
  };

} // namespace art

#endif /* art_Framework_Services_Registry_ServiceRegistry_h */

// Local Variables:
// mode: c++
// End:
