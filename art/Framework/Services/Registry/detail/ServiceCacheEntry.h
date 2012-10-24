#ifndef art_Framework_Services_Registry_detail_ServiceCacheEntry_h
#define art_Framework_Services_Registry_detail_ServiceCacheEntry_h
////////////////////////////////////////////////////////////////////////
// ServiceCacheEntry
//
// Used by ServicesManager to handle creation and caching of services.

#include "art/Utilities/LibraryManager.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceCache.h"
#include "art/Framework/Services/Registry/detail/ServiceStack.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"
#include "cetlib/demangle.h"
#include "cetlib/trim.h"
#include "cpp0x/memory"
#include "cpp0x/utility"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <cassert>
#include <iomanip>
#include <map>
#include <stack>
#include <string>
#include <vector>

namespace art {
  namespace detail {
    class ServiceCacheEntry;
  }
}

// A ServiceCacheEntry object encapsulates the (possibly delayed)
// creation of a Service object.
class art::detail::ServiceCacheEntry {
public:
  // Service implementation.
ServiceCacheEntry(fhicl::ParameterSet const & pset,
                  std::unique_ptr<detail::ServiceHelperBase> && helper)
  :
  config_(pset),
    helper_(std::move(helper)),
    service_(),
    interface_impl_()
    {
    }

  // Service interface implementation.
ServiceCacheEntry(fhicl::ParameterSet const & pset,
                  std::unique_ptr<detail::ServiceHelperBase> && helper,
                  ServiceCache::iterator impl)
  :
  config_(pset),
    helper_(std::move(helper)),
    service_(),
    interface_impl_(impl)
    {
    }

  // Pre-made service (1).
ServiceCacheEntry(WrapperBase_ptr premade_service,
                  std::unique_ptr<detail::ServiceHelperBase> && helper)
  :
  config_(),
    helper_(std::move(helper)),
    service_(premade_service),
    interface_impl_()
    {
    }

  // Create the service if necessary, and return the WrapperBase_ptr
  // that refers to it.
  WrapperBase_ptr getService(ActivityRegistry & reg, detail::ServiceStack & creationOrder) const {
    if (is_interface()) { // Service interface
      if (!service_) {
        // No cached instance, we need to make it.
        if (!interface_impl_->second.service_) {
          // The service provider has no cached instance, have it make one.
          interface_impl_->second.createService(reg, creationOrder);
        }
        // Convert the service provider wrapper to a service interface wrapper,
        // and use that as our cached instance.
        interface_impl_->second.convertService(service_);
      }
    }
    else { // Concrete service.
      if (!service_) {
        // No cached instance, we need to make it.
        createService(reg, creationOrder);
      }
    }
    return service_;
  }

  void forceCreation(ActivityRegistry & reg) const {
    assert(is_impl() && "ServiceCacheEntry::forceCreation called on a service interface!");
    if (!service_) {
      makeAndCacheService(reg);
    }
  }

  fhicl::ParameterSet const & getParameterSet() const {
    return config_;
  }

  void putParameterSet(fhicl::ParameterSet const & newConfig) {
    if (config_ != newConfig) {
      config_ = newConfig;
      if (service_) {
        service_->reconfigure(config_);
      }
    }
  }

  ServiceCache::iterator getInterfaceImpl() const {
    return interface_impl_;
  }

  void makeAndCacheService(ActivityRegistry & reg) const {
    assert(is_impl() && "ServiceCacheEntry::makeAndCacheService called on a service interface!");
    try {
      if (serviceScope() == ServiceScope::PER_SCHEDULE) {
        service_ = dynamic_cast<detail::ServicePSMHelper&>(*helper_).make(config_, reg, nSchedules());
      } else {
        service_ = dynamic_cast<detail::ServiceLGMHelper&>(*helper_).make(config_, reg);
      }
    }
    catch (cet::exception & e) {
      throw Exception(errors::OtherArt, "ServiceCreation", e)
        << "cet::exception caught during construction of service type "
        << cet::demangle_symbol(helper_->get_typeid().name())
        << ":\n";
    }
    catch (std::exception & e) {
      throw Exception(errors::StdException, "ServiceCreation")
        << "std::exception caught during construction of service type "
        << cet::demangle_symbol(helper_->get_typeid().name())
        << ": "
        << e.what();
    }
    catch (std::string const & s) {
      throw Exception(errors::BadExceptionType)
        << "String exception during construction of service type "
        << cet::demangle_symbol(helper_->get_typeid().name())
        << ": "
        << s;
    }
    catch (...) {
      throw Exception(errors::Unknown)
        << "String exception during construction of service type "
        << cet::demangle_symbol(helper_->get_typeid().name())
        << ":\n";
    }
  }

  void
    createService(ActivityRegistry & reg, detail::ServiceStack & creationOrder) const {
    assert(is_impl() && "ServiceCacheEntry::createService called on a service interface!");
    // When we actually create the Service object, we have to
    // remember the order of creation.
    makeAndCacheService(reg);
    creationOrder.push(service_);
  }

  void
    convertService(WrapperBase_ptr & swb) const {
    assert(is_impl() && "ServiceCacheEntry::convertService called on a service interface!");
    swb = dynamic_cast<detail::ServiceInterfaceImplHelper&>(*helper_).convert(service_);
  }

  ServiceScope serviceScope() const { return helper_->scope(); }

  template <typename T,
    typename = typename std::enable_if<detail::ServiceHelper<T>::scope_val != ServiceScope::PER_SCHEDULE>::type>
    T & get(ActivityRegistry & reg, detail::ServiceStack & creationOrder) const {
    assert(serviceScope() != ServiceScope::PER_SCHEDULE &&
           "Called wrong service get() function: need ScheduleID");
    WrapperBase_ptr swb = getService(reg, creationOrder);
    return *reinterpret_cast<T *>(dynamic_cast<detail::ServiceLGRHelper&>(*helper_).retrieve(swb));
  }

  template <typename T,
    typename = typename std::enable_if<detail::ServiceHelper<T>::scope_val == ServiceScope::PER_SCHEDULE>::type>
    T & get(ActivityRegistry & reg,
            detail::ServiceStack & creationOrder,
            ScheduleID sID) const {
    assert(serviceScope() == ServiceScope::PER_SCHEDULE &&
           "Called wrong service get() function!");
    WrapperBase_ptr swb = getService(reg, creationOrder);
    return *reinterpret_cast<T *>(dynamic_cast<detail::ServicePSRHelper&>(*helper_).retrieve(swb, sID));
  }

  static void setNSchedules(size_t nSched) { nSchedules() = nSched; }

private:
  static size_t & nSchedules() { static size_t s_ns = 1; return s_ns; }

  bool is_impl() const { return !is_interface(); }
  bool is_interface() const { return helper_->is_interface(); }

  fhicl::ParameterSet config_;
  std::unique_ptr<detail::ServiceHelperBase> helper_;
  mutable WrapperBase_ptr service_;
  ServiceCache::iterator const interface_impl_;
};  // ServiceCacheEntry

#endif /* art_Framework_Services_Registry_detail_ServiceCacheEntry_h */

// Local Variables:
// mode: c++
// End:
