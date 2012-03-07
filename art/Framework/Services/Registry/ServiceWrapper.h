#ifndef art_Framework_Services_Registry_ServiceWrapper_h
#define art_Framework_Services_Registry_ServiceWrapper_h

// ======================================================================
//
// ServiceWrapper - Class template through which the framework manages
//                  the lifetime of a service.
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/ServiceWrapperBase.h"
#include "art/Utilities/detail/metaprogramming.h"
#include "art/Utilities/ScheduleID.h"
#include "cpp0x/memory"
#include "cpp0x/type_traits"

namespace art {
  template <typename T, ServiceScope SCOPE = ServiceScope::GLOBAL>
  class ServiceWrapper;

  // Partially specialized templates.
  template <typename T>
  class ServiceWrapper<T, ServiceScope::GLOBAL>;

  template <typename T>
  class ServiceWrapper<T, ServiceScope::PER_SCHEDULE>;

  namespace detail {
    template <typename T, void (T:: *)(fhicl::ParameterSet const &)>  struct reconfig_function;
    template <typename T> no_tag  has_reconfig_helper(...);
    template <typename T> yes_tag has_reconfig_helper(reconfig_function<T, &T::reconfigure> * dummy);

    template<typename T>
    struct has_reconfig_function {
      static bool const value =
        sizeof(has_reconfig_helper<T>(0)) == sizeof(yes_tag);
    };

    template <typename T>
    struct DoReconfig {
      void operator()(T & a, fhicl::ParameterSet const & b) { a.reconfigure(b); }
    };

    template <typename T>
    struct DoNothing {
      void operator()(T &, fhicl::ParameterSet const &) { }
    };
  }
}

template <typename T>
class art::ServiceWrapper<T, art::ServiceScope::GLOBAL>
    : public ServiceWrapperBase {
  // Non-copyable:
  ServiceWrapper(ServiceWrapper const &) = delete;
  void operator = (ServiceWrapper const &) = delete;

public:
  explicit ServiceWrapper(std::auto_ptr<T> service_ptr)
    : ServiceWrapperBase()
    , service_ptr_(service_ptr)          // take ownership
  { }

  // accessor:
  T & get() const { return *service_ptr_; }

private:
  void reconfigure_service(fhicl::ParameterSet const & n) {
    typename std::conditional < detail::has_reconfig_function<T>::value,
             detail::DoReconfig<T>,
             detail::DoNothing<T> >::type reconfig_or_nothing;
    reconfig_or_nothing(*service_ptr_, n);
  }

  ServiceScope service_scope() const { return ServiceScope::GLOBAL; }

  std::auto_ptr<T> service_ptr_;
};

template <typename T>
class art::ServiceWrapper<T, art::ServiceScope::PER_SCHEDULE>
    : public ServiceWrapperBase {
  // Non-copyable:
  ServiceWrapper(ServiceWrapper const &) = delete;
  void operator = (ServiceWrapper const &) = delete;

public:
  explicit ServiceWrapper(std::vector<std::unique_ptr<T>> && service_ptrs)
    :
    ServiceWrapperBase(),
    service_ptrs_(std::move(service_ptrs)) {
  }

  T & get(ScheduleID sID) const { return *service_ptrs_.at(sID.id()); }

private:
  void reconfigure_service(fhicl::ParameterSet const & n) {
    typename std::conditional < detail::has_reconfig_function<T>::value,
             detail::DoReconfig<T>,
             detail::DoNothing<T> >::type reconfig_or_nothing;
  for (auto & sPtr : service_ptrs_) {
      reconfig_or_nothing(*sPtr, n);
    }
  }

  ServiceScope service_scope() const { return ServiceScope::PER_SCHEDULE; }

  std::vector<std::unique_ptr<T>> service_ptrs_;
};

// ======================================================================

#endif /* art_Framework_Services_Registry_ServiceWrapper_h */

// Local Variables:
// mode: c++
// End:
