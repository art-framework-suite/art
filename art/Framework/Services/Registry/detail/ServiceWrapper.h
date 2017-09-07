#ifndef art_Framework_Services_Registry_detail_ServiceWrapper_h
#define art_Framework_Services_Registry_detail_ServiceWrapper_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "art/Utilities/ScheduleID.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/metaprogramming.h"

#include <memory>
#include <type_traits>

namespace art {

class ActivityRegistry;

namespace detail {

using cet::enable_if_function_exists_t;

template <typename T, ServiceScope SCOPE>
class ServiceWrapper;

// If we have a constructor taking fhicl::ParameterSet const& and
// ActivityRegistry&, use it. Otherwise, call a one-argument
// constructor taking fhicl::ParameterSet const& only.
template <typename T>
std::enable_if_t<std::is_constructible<T, fhicl::ParameterSet const&, ActivityRegistry&>::value, std::shared_ptr<T>>
makeServiceFrom(fhicl::ParameterSet const& ps, ActivityRegistry& areg)
{
  return std::make_shared<T>(ps, areg);
}

template <typename T>
std::enable_if_t<!std::is_constructible<T, fhicl::ParameterSet const&, ActivityRegistry&>::value, std::shared_ptr<T>>
makeServiceFrom(fhicl::ParameterSet const& ps, ActivityRegistry&)
{
  return std::make_shared<T>(ps);
}

extern "C"
std::unique_ptr<ServiceWrapperBase>
converter(std::shared_ptr<ServiceWrapperBase> const&);

template <typename T, art::ServiceScope SCOPE>
class ServiceWrapper : public ServiceWrapperBase {

public:

  ServiceWrapper(ServiceWrapper const&) = delete;
  ServiceWrapper& operator=(ServiceWrapper const&) = delete;

  ServiceWrapper(fhicl::ParameterSet const& ps, ActivityRegistry& areg)
    : service_ptr_{makeServiceFrom<T>(ps, areg)}
  {
  }

  explicit
  ServiceWrapper(std::shared_ptr<T>&& p)
    : service_ptr_{std::move(p)}
  {
  }

  T&
  get()
  {
    return *service_ptr_;
  }

  template <typename U, typename = std::enable_if_t<std::is_base_of<U, T>::value>>
  ServiceWrapper<U, SCOPE>*
  getAs() const
  {
    return new ServiceWrapper<U, SCOPE>{std::static_pointer_cast<U>(service_ptr_)};
  }

private:

  std::shared_ptr<T>
  service_ptr_;

};

} // namespace detail
} // namespace art

#endif /* art_Framework_Services_Registry_detail_ServiceWrapper_h */

// Local Variables:
// mode: c++
// End:
