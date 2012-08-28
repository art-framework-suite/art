#ifndef art_Framework_Services_Registry_ServiceWrapper_h
#define art_Framework_Services_Registry_ServiceWrapper_h

// ======================================================================
//
// ServiceWrapper - Class template through which the framework manages
//                  the lifetime of a service.
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceWrapperBase.h"
#include "art/Utilities/detail/metaprogramming.h"
#include "cpp0x/memory"
#include "cpp0x/type_traits"

namespace art {

  template<class T> class ServiceWrapper;

  namespace detail {

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
extern "C" std::unique_ptr<art::ServiceWrapperBase> converter(std::shared_ptr<art::ServiceWrapperBase> const &);

template<typename T>
class art::ServiceWrapper : public ServiceWrapperBase
{

  // let the service interface macro access me
  friend std::unique_ptr<art::ServiceWrapperBase> (::converter)(std::shared_ptr<art::ServiceWrapperBase> const &);

  // non-copyable:
  ServiceWrapper(ServiceWrapper const &);
  void operator=(ServiceWrapper const &);

public:

  // c'tor from a unique_ptr:
  explicit ServiceWrapper(std::unique_ptr<T> && p)
    : ServiceWrapperBase(), service_ptr_(std::move(p))
  {
  }

  // c'tor from shared_ptr:
  explicit ServiceWrapper(std::shared_ptr<T> && p)
    : ServiceWrapperBase(), service_ptr_(std::move(p))
  {
  }

  // use compiler-generated (virtual) d'tor

  // accessor:
  T & get() const
  {
    return *service_ptr_;
  }

  void reconfigure(fhicl::ParameterSet const & n)
  {
    typename std::conditional<detail::has_reconfig_function<T>::value,
      detail::DoReconfig<T>,
      detail::DoNothing<T> >::type reconfig_or_nothing;
    reconfig_or_nothing(*service_ptr_,n);
  }

private:

  // convert
  template<typename U, typename = typename std::enable_if<std::is_base_of<U,T>::value>::type>
  ServiceWrapper<U> * getAs() const
  {
    return new ServiceWrapper<U>(std::static_pointer_cast<U>(service_ptr_));
  }
  
  std::shared_ptr<T> service_ptr_;

}; // ServiceWrapper<T>

// ======================================================================

#endif // art_Framework_Services_Registry_ServiceWrapper_h

// Local Variables:
// mode: c++
// End:
