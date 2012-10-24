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
#include "art/Utilities/detail/metaprogramming.h"
#include "cpp0x/memory"
#include "cpp0x/type_traits"

namespace art {
// Forward declaration
  class ActivityRegistry;

  namespace detail {

    // General template.
    template<typename T, ServiceScope SCOPE>
    class ServiceWrapper;

    // Partial specialization.
    template <typename T>
    class ServiceWrapper<T, ServiceScope::PER_SCHEDULE>;

////////////////////////////////////
    // Support structures.
    template<typename T, void (T::*)(fhicl::ParameterSet const &)>  struct reconfig_function;
    template<typename T> no_tag  has_reconfig_helper(...);
    template<typename T> yes_tag has_reconfig_helper(reconfig_function<T, &T::reconfigure> * dummy);

    template<typename T>
    struct has_reconfig_function
    {
      static bool const value = sizeof(has_reconfig_helper<T>(0)) == sizeof(yes_tag);
    };

    template<typename T>
    struct DoReconfig
    {
      void operator()(T & a, fhicl::ParameterSet const & b) { a.reconfigure(b); }
    };

    template <typename T>
    struct DoNothing
    {
      void operator()(T &, fhicl::ParameterSet const &) {}
    };

  } // namespace detail

} // namespace art



// ----------------------------------------------------------------------

// declare the linkage before the friend declaration
extern "C" std::unique_ptr<art::detail::ServiceWrapperBase> converter(std::shared_ptr<art::detail::ServiceWrapperBase> const &);

// General template.
template<typename T, art::ServiceScope SCOPE>
class art::detail::ServiceWrapper : public ServiceWrapperBase
{
public:
  // Non-copyable.
  ServiceWrapper(ServiceWrapper const &) = delete;
  void operator=(ServiceWrapper const &) = delete;

  // C'tor from ParameterSet, ActivityRegistry.
  ServiceWrapper(fhicl::ParameterSet const & ps,
                 ActivityRegistry & areg)
    :
    ServiceWrapperBase(),
    service_ptr_(new T(ps, areg))
    {
    }

  // C'tor from shared_ptr.
  explicit ServiceWrapper(std::shared_ptr<T> && p)
    : ServiceWrapperBase(), service_ptr_(std::move(p))
    {
    }

  T & get() { return *service_ptr_; }

  template<typename U, typename = typename std::enable_if<std::is_base_of<U,T>::value>::type>
  ServiceWrapper<U, SCOPE> * getAs() const
    {
      return new ServiceWrapper<U, SCOPE>(std::static_pointer_cast<U>(service_ptr_));
    }

private:
  void reconfigure_service(fhicl::ParameterSet const & n) override
    {
      typename std::conditional<detail::has_reconfig_function<T>::value,
        detail::DoReconfig<T>,
        detail::DoNothing<T> >::type reconfig_or_nothing;
      reconfig_or_nothing(*service_ptr_,n);
    }

  std::shared_ptr<T> service_ptr_;

}; // ServiceWrapper<T, ServiceScope>

// Partially-specialized template.
template <typename T>
class art::detail::ServiceWrapper<T, art::ServiceScope::PER_SCHEDULE> : public ServiceWrapperBase {
public:
  // Non-copyable.
  ServiceWrapper(ServiceWrapper const &) = delete;
  void operator=(ServiceWrapper const &) = delete;

  // C'tor from shared_ptrs.
  explicit ServiceWrapper(std::vector<std::shared_ptr<T>> && service_ptrs)
    :
    ServiceWrapperBase(),
    service_ptrs_(std::move(service_ptrs))
    {
    }

  // C'tor from collection of convertible-to-shared-ptr
  template <class SP>
  explicit ServiceWrapper(std::vector<SP> && service_ptrs)
    :
    ServiceWrapperBase(),
    service_ptrs_()
    {
      service_ptrs_.reserve(service_ptrs.size());
      for (auto && up : service_ptrs) {
        service_ptrs_.emplace_back(std::move(up));
      }
    }

  // C'tor from ParameterSet, ActivityRegistry, nSchedules.
  ServiceWrapper(fhicl::ParameterSet const & ps,
                 ActivityRegistry & areg,
                 size_t nSchedules)
    :
    ServiceWrapperBase(),
    service_ptrs_()
    {
      service_ptrs_.reserve(nSchedules);
      ScheduleID id(ScheduleID::min_id());
      size_t iSched(0);
      for (; iSched < nSchedules; ++iSched, id = id.next()) {
        service_ptrs_.emplace_back(new T(ps,
                                         areg,
                                         id));
      }
    }

  T & get(ScheduleID sID) { return *service_ptrs_.at(sID.id()); }

  template<typename U, typename = typename std::enable_if<std::is_base_of<U,T>::value>::type>
  ServiceWrapper<U, art::ServiceScope::PER_SCHEDULE> * getAs() const
    {
      std::vector<std::shared_ptr<U>> converted_ptrs(service_ptrs_.size());
      std::transform(service_ptrs_.begin(),
                     service_ptrs_.end(),
                     converted_ptrs.begin(),
                     [](std::shared_ptr<T> const & ptr_in) {
                       return std::static_pointer_cast<U>(ptr_in);
                     });
      return new ServiceWrapper<U, art::ServiceScope::PER_SCHEDULE>(std::move(converted_ptrs));
    }

private:
  void reconfigure_service(fhicl::ParameterSet const & n) override
    {
      typename std::conditional<detail::has_reconfig_function<T>::value,
        detail::DoReconfig<T>,
        detail::DoNothing<T> >::type reconfig_or_nothing;
      for (auto & service_ptr : service_ptrs_)
      {
        reconfig_or_nothing(*service_ptr, n);
      }
    }

  std::vector<std::shared_ptr<T>> service_ptrs_;

};


// ======================================================================

#endif /* art_Framework_Services_Registry_detail_ServiceWrapper_h */

// Local Variables:
// mode: c++
// End:
