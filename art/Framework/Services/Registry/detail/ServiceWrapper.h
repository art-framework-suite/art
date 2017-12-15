#ifndef art_Framework_Services_Registry_detail_ServiceWrapper_h
#define art_Framework_Services_Registry_detail_ServiceWrapper_h

// ======================================================================
//
// ServiceWrapper - Class template through which the framework manages
//                  the lifetime of a service.
//
// ======================================================================

#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Utilities/ScheduleID.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/metaprogramming.h"
#include <memory>
#include <type_traits>

namespace art {
  // Forward declaration
  class ActivityRegistry;
  class ModuleDescription;
  class ProducingService;

  namespace detail {

    using cet::enable_if_function_exists_t;

    // General template.
    template <typename T, ServiceScope SCOPE>
    class ServiceWrapper;

    // Partial specialization.
    template <typename T>
    class ServiceWrapper<T, ServiceScope::PER_SCHEDULE>;

    // If we have a constructor taking fhicl::ParameterSet const& and
    // ActivityRegistry&, use it. Otherwise, call a one-argument
    // constructor taking fhicl::ParameterSet const& only.
    template <typename T>
    std::enable_if_t<std::is_constructible<T,
                                           fhicl::ParameterSet const&,
                                           ActivityRegistry&>::value,
                     std::shared_ptr<T>>
    makeServiceFrom(fhicl::ParameterSet const& ps, ActivityRegistry& areg)
    {
      static_assert(
        !std::is_base_of<ProducingService, T>::value,
        "\n\nart-error: A service that inherits from art::ProducingService\n"
        "           cannot have a constructor that takes an ActivityRegistry&\n"
        "           argument.  Contact artists@fnal.gov for guidance.\n");
      return std::make_shared<T>(ps, areg);
    }

    template <typename T>
    std::enable_if_t<!std::is_constructible<T,
                                            fhicl::ParameterSet const&,
                                            ActivityRegistry&>::value,
                     std::shared_ptr<T>>
    makeServiceFrom(fhicl::ParameterSet const& ps, ActivityRegistry&)
    {
      return std::make_shared<T>(ps);
    }

  } // namespace detail
} // namespace art

// ----------------------------------------------------------------------

// declare the linkage before the friend declaration
extern "C" std::unique_ptr<art::detail::ServiceWrapperBase> converter(
  std::shared_ptr<art::detail::ServiceWrapperBase> const&);

// General template.
template <typename T, art::ServiceScope SCOPE>
class art::detail::ServiceWrapper : public ServiceWrapperBase {
public:
  // Non-copyable.
  ServiceWrapper(ServiceWrapper const&) = delete;
  ServiceWrapper& operator=(ServiceWrapper const&) = delete;

  // C'tor from ParameterSet, ActivityRegistry.
  ServiceWrapper(fhicl::ParameterSet const& ps, ActivityRegistry& areg)
    : service_ptr_{makeServiceFrom<T>(ps, areg)}
  {}

  // C'tor from shared_ptr.
  explicit ServiceWrapper(std::shared_ptr<T>&& p) : service_ptr_{std::move(p)}
  {}

  T&
  get()
  {
    return *service_ptr_;
  }

  template <typename U,
            typename = std::enable_if_t<std::is_base_of<U, T>::value>>
  ServiceWrapper<U, SCOPE>*
  getAs() const
  {
    return new ServiceWrapper<U, SCOPE>{
      std::static_pointer_cast<U>(service_ptr_)};
  }

private:
  template <typename U = T>
  std::enable_if_t<std::is_base_of<ProducingService, U>::value>
  doRegisterProducts(MasterProductRegistry& mpr,
                     ProductDescriptions& productsToProduce,
                     ProducingServiceSignals& signals,
                     ModuleDescription const& md)
  {
    service_ptr_->registerCallbacks(signals);
    service_ptr_->setModuleDescription(md);
    service_ptr_->registerProducts(mpr, productsToProduce, md);
  }

  template <typename U = T>
  std::enable_if_t<!std::is_base_of<ProducingService, U>::value>
  doRegisterProducts(MasterProductRegistry&,
                     ProductDescriptions&,
                     ProducingServiceSignals&,
                     ModuleDescription const&)
  {}

  void
  registerProducts(MasterProductRegistry& mpr,
                   ProductDescriptions& productsToProduce,
                   ProducingServiceSignals& signals,
                   ModuleDescription const& md) override
  {
    doRegisterProducts(mpr, productsToProduce, signals, md);
  }

  std::shared_ptr<T> service_ptr_;

}; // ServiceWrapper<T, ServiceScope>

// Partially-specialized template.
template <typename T>
class art::detail::ServiceWrapper<T, art::ServiceScope::PER_SCHEDULE>
  : public ServiceWrapperBase {
public:
  // Non-copyable.
  ServiceWrapper(ServiceWrapper const&) = delete;
  void operator=(ServiceWrapper const&) = delete;

  // C'tor from shared_ptrs.
  explicit ServiceWrapper(std::vector<std::shared_ptr<T>>&& service_ptrs)
    : service_ptrs_{std::move(service_ptrs)}
  {}

  // C'tor from collection of convertible-to-shared-ptr
  template <class SP>
  explicit ServiceWrapper(std::vector<SP>&& service_ptrs)
  {
    service_ptrs_.reserve(service_ptrs.size());
    for (auto&& up : service_ptrs) {
      service_ptrs_.emplace_back(std::move(up));
    }
  }

  // C'tor from ParameterSet, ActivityRegistry, nSchedules.
  ServiceWrapper(fhicl::ParameterSet const& ps,
                 ActivityRegistry& areg,
                 size_t const nSchedules)
  {
    service_ptrs_.reserve(nSchedules);
    ScheduleID id{ScheduleID::first()};
    for (size_t iSched{}; iSched < nSchedules; ++iSched, id = id.next()) {
      service_ptrs_.emplace_back(new T{ps, areg, id});
    }
  }

  T&
  get(ScheduleID sID)
  {
    return *service_ptrs_.at(sID.id());
  }

  template <typename U,
            typename = std::enable_if_t<std::is_base_of<U, T>::value>>
  ServiceWrapper<U, art::ServiceScope::PER_SCHEDULE>*
  getAs() const
  {
    std::vector<std::shared_ptr<U>> converted_ptrs(service_ptrs_.size());
    cet::transform_all(service_ptrs_,
                       converted_ptrs.begin(),
                       [](std::shared_ptr<T> const& ptr_in) {
                         return std::static_pointer_cast<U>(ptr_in);
                       });
    return new ServiceWrapper<U, art::ServiceScope::PER_SCHEDULE>(
      std::move(converted_ptrs));
  }

private:
  // No products can be registered for per-schedule services.
  void
  registerProducts(MasterProductRegistry&,
                   ProductDescriptions&,
                   ProducingServiceSignals&,
                   ModuleDescription const&) override
  {}

  std::vector<std::shared_ptr<T>> service_ptrs_{};
};

// ======================================================================

#endif /* art_Framework_Services_Registry_detail_ServiceWrapper_h */

// Local Variables:
// mode: c++
// End:
