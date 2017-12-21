#include "art/Framework/Services/Registry/detail/ServiceCacheEntry.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Framework/Services/Registry/detail/ServiceStack.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "art/Utilities/bold_fontify.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/detail/validationException.h"

#include <cassert>
#include <memory>
#include <string>
#include <utility>

using namespace std;

namespace art {
  namespace detail {

    ServiceCacheEntry::ServiceCacheEntry(fhicl::ParameterSet const& pset,
                                         unique_ptr<ServiceHelperBase>&& helper)
      : config_{pset}, helper_{move(helper)}
    {}

    ServiceCacheEntry::ServiceCacheEntry(fhicl::ParameterSet const& pset,
                                         unique_ptr<ServiceHelperBase>&& helper,
                                         ServiceCacheEntry const& impl)
      : config_{pset}, helper_{move(helper)}, interface_impl_{&impl}
    {}

    ServiceCacheEntry::ServiceCacheEntry(
      shared_ptr<ServiceWrapperBase> premade_service,
      unique_ptr<ServiceHelperBase>&& helper)
      : helper_{move(helper)}, service_{premade_service}
    {}

    shared_ptr<ServiceWrapperBase>
    ServiceCacheEntry::getService(art::ActivityRegistry& reg,
                                  ServiceStack& creationOrder) const
    {
      if (is_interface()) {
        if (!service_) {
          // No cached instance, we need to make it.
          if (!interface_impl_->service_) {
            // The service provider has no cached instance, have it make one.
            interface_impl_->createService(reg, creationOrder);
          }
          // Convert the service provider wrapper to a service interface
          // wrapper, and use that as our cached instance.
          interface_impl_->convertService(service_);
        }
        return service_;
      }
      if (!service_) {
        // No cached instance, we need to make it.
        createService(reg, creationOrder);
      }
      return service_;
    }

    void
    ServiceCacheEntry::makeAndCacheService(art::ActivityRegistry& reg) const
    {
      assert(is_impl() && "ServiceCacheEntry::makeAndCacheService called on a "
                          "service interface!");
      try {
        service_ = dynamic_cast<ServiceLGMHelper&>(*helper_).make(config_, reg);
      }
      catch (fhicl::detail::validationException const& e) {
        std::ostringstream err_stream;
        constexpr cet::HorizontalRule rule{100};
        err_stream << "\n"
                   << rule('=') << "\n\n"
                   << "!! The following service has been misconfigured: !!"
                   << "\n\n"
                   << rule('-') << "\n\nservice_type: "
                   << art::detail::bold_fontify(
                        config_.get<std::string>("service_type"))
                   << "\n\n"
                   << e.what() << "\n"
                   << rule('=') << "\n\n";
        throw art::Exception(art::errors::Configuration) << err_stream.str();
      }
      catch (cet::exception& e) {
        throw Exception(errors::OtherArt, "ServiceCreation", e)
          << "cet::exception caught during construction of service type "
          << cet::demangle_symbol(helper_->get_typeid().name()) << ":\n";
      }
      catch (exception& e) {
        throw Exception(errors::StdException, "ServiceCreation")
          << "exception caught during construction of service type "
          << cet::demangle_symbol(helper_->get_typeid().name()) << ": "
          << e.what();
      }
      catch (string const& s) {
        throw Exception(errors::BadExceptionType)
          << "String exception during construction of service type "
          << cet::demangle_symbol(helper_->get_typeid().name()) << ": " << s;
      }
      catch (...) {
        throw Exception(errors::Unknown)
          << "String exception during construction of service type "
          << cet::demangle_symbol(helper_->get_typeid().name()) << ":\n";
      }
    }

    void
    ServiceCacheEntry::forceCreation(art::ActivityRegistry& reg) const
    {
      assert(is_impl() &&
             "ServiceCacheEntry::forceCreation called on a service interface!");
      if (!service_) {
        makeAndCacheService(reg);
      }
    }

    void
    ServiceCacheEntry::registerProducts(ProductDescriptions& productsToProduce,
                                        ProducingServiceSignals& signals,
                                        ModuleDescription const& md)
    {
      service_->registerProducts(productsToProduce, signals, md);
    }

    fhicl::ParameterSet const&
    ServiceCacheEntry::getParameterSet() const
    {
      return config_;
    }

    void
    ServiceCacheEntry::createService(art::ActivityRegistry& reg,
                                     ServiceStack& creationOrder) const
    {
      assert(is_impl() &&
             "ServiceCacheEntry::createService called on a service interface!");
      // When we actually create the Service object, we have to
      // remember the order of creation.
      makeAndCacheService(reg);
      creationOrder.push(service_);
    }

    void
    ServiceCacheEntry::convertService(shared_ptr<ServiceWrapperBase>& swb) const
    {
      assert(
        is_impl() &&
        "ServiceCacheEntry::convertService called on a service interface!");
      swb =
        dynamic_cast<ServiceInterfaceImplHelper&>(*helper_).convert(service_);
    }

    ServiceScope
    ServiceCacheEntry::serviceScope() const
    {
      return helper_->scope();
    }

    bool
    ServiceCacheEntry::is_interface() const
    {
      return helper_->is_interface();
    }

    bool
    ServiceCacheEntry::is_impl() const
    {
      return !is_interface();
    }

  } // namespace detail
} // namespace art
