////////////////////////////////////////////////////////////////////////
// ServiceCacheEntry
//
// Used by ServicesManager to handle creation and caching of services.

#include "art/Framework/Services/Registry/detail/ServiceCacheEntry.h"

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceStack.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/demangle.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/detail/validationException.h"

#include <cassert>
#include <memory>
#include <string>
#include <utility>

art::detail::ServiceCacheEntry::
ServiceCacheEntry(fhicl::ParameterSet const & pset,
                  std::unique_ptr<detail::ServiceHelperBase> && helper)
  :
  config_(pset),
  helper_(std::move(helper)),
  service_(),
  interface_impl_(nullptr)
{
}

// Service interface implementation.
art::detail::ServiceCacheEntry::
ServiceCacheEntry(fhicl::ParameterSet const & pset,
                  std::unique_ptr<detail::ServiceHelperBase> && helper,
                  ServiceCacheEntry const & impl)
  :
  config_(pset),
  helper_(std::move(helper)),
  service_(),
  interface_impl_(&impl)
{
}

// Pre-made service (1).
art::detail::ServiceCacheEntry::
ServiceCacheEntry(WrapperBase_ptr premade_service,
                  std::unique_ptr<detail::ServiceHelperBase> && helper)
  :
  config_(),
  helper_(std::move(helper)),
  service_(premade_service),
  interface_impl_(nullptr)
{
}

// Create the service if necessary, and return the WrapperBase_ptr that
// refers to it.
art::detail::WrapperBase_ptr
art::detail::ServiceCacheEntry::
getService(ActivityRegistry & reg, detail::ServiceStack & creationOrder) const
{
  if (is_interface()) { // Service interface
    if (!service_) {
      // No cached instance, we need to make it.
      if (!interface_impl_->service_) {
        // The service provider has no cached instance, have it make one.
        interface_impl_->createService(reg, creationOrder);
      }
      // Convert the service provider wrapper to a service interface wrapper,
      // and use that as our cached instance.
      interface_impl_->convertService(service_);
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

void
art::detail::ServiceCacheEntry::
putParameterSet(fhicl::ParameterSet const & newConfig)
{
  if (config_ != newConfig) {
    config_ = newConfig;
    if (service_) {
      service_->reconfigure(config_);
    }
  }
}

void
art::detail::ServiceCacheEntry::
makeAndCacheService(ActivityRegistry & reg) const
{
  assert(is_impl() && "ServiceCacheEntry::makeAndCacheService called on a service interface!");
  try {
    if (serviceScope() == ServiceScope::PER_SCHEDULE) {
      service_ = dynamic_cast<detail::ServicePSMHelper &>(*helper_).make(config_, reg, nSchedules());
    }
    else {
      service_ = dynamic_cast<detail::ServiceLGMHelper &>(*helper_).make(config_, reg);
    }
  }
  catch (fhicl::detail::validationException const & e)
    {
      std::ostringstream err_stream;
      std::size_t const width (100);
      err_stream << "\n"
                 << std::string(width,'=')
                 << "\n\n"
                 << "!! The following service has been misconfigured: !!"
                 << "\n\n"
                 << std::string(width,'-')
                 << "\n\nservice_type: \033[1m" << config_.get<std::string>("service_type") << "\033[0m"
                 << "\n\n" << e.what()
                 << "\n"
                 << std::string(width,'=')
                 << "\n\n";
      throw art::Exception(art::errors::Configuration) << err_stream.str();
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
