#ifndef art_Framework_Services_Registry_ServiceWrapper_h
#define art_Framework_Services_Registry_ServiceWrapper_h

// ======================================================================
//
// ServiceWrapper - Class template through which the framework manages
//                  the lifetime of a service.
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceWrapperBase.h"

#include "boost/mpl/if.hpp"
#include <memory>

namespace art {
  template< class T >
    class ServiceWrapper;

  namespace detail {
    typedef char (& no_tag)[1]; // type indicating FALSE
    typedef char (& yes_tag)[2]; // type indicating TRUE
    
    template <typename T, void (T::*)(fhicl::ParameterSet const&)>  struct reconfig_function;
    template <typename T> no_tag  has_reconfig_helper(...);
    template <typename T> yes_tag has_reconfig_helper(reconfig_function<T, &T::reconfigure> * dummy);
    
    template<typename T>
    struct has_reconfig_function
    {
      static bool const value =
        sizeof(has_reconfig_helper<T>(0)) == sizeof(yes_tag);
    };
    
    template <typename T>
    struct DoReconfig
    {
      void operator()(T& a, fhicl::ParameterSet const& b) { a.reconfigure(b); }
    };
    
    template <typename T>
    struct DoNothing
    {
      void operator()(T& a, fhicl::ParameterSet const& b) { }
    };
  }
}



// ----------------------------------------------------------------------

template< class T >
class art::ServiceWrapper
  : public ServiceWrapperBase
{
  // non-copyable:
  ServiceWrapper( ServiceWrapper const & );
  void operator = ( ServiceWrapper const & );

public:
  // c'tor:
  explicit ServiceWrapper( std::auto_ptr<T> service_ptr )
  : ServiceWrapperBase( )
  , service_ptr_      ( service_ptr )  // take ownership
  { }

  // use compiler-generated (virtual) d'tor

  // accessor:
  T &
    get() const { return *service_ptr_; }

  void reconfigure(fhicl::ParameterSet const& n)
  {
    typename boost::mpl::if_c<detail::has_reconfig_function<T>::value,
      detail::DoReconfig<T>,
      detail::DoNothing<T> >::type reconfig_or_nothing;
    reconfig_or_nothing(*service_ptr_,n);
  }

private:
  std::auto_ptr<T> service_ptr_;

};  // ServiceWrapper<>

// ======================================================================

#endif /* art_Framework_Services_Registry_ServiceWrapper_h */

// Local Variables:
// mode: c++
// End:
