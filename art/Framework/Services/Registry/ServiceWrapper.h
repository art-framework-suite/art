#ifndef ServiceRegistry_ServiceWrapper_h
#define ServiceRegistry_ServiceWrapper_h

// ======================================================================
//
// ServiceWrapper - Class template through which the framework manages
//                  the lifetime of a service.
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceWrapperBase.h"
#include <memory>  // auto_ptr

namespace art { namespace serviceregistry {
  template< class T >
    class ServiceWrapper;
} }

// ----------------------------------------------------------------------

template< class T >
class art::serviceregistry::ServiceWrapper
  : public ServiceWrapperBase
{
  // non-copyable:
  ServiceWrapper( ServiceWrapper const & );
  void operator = ( ServiceWrapper const & );

public:
  // c'tor:
  explicit ServiceWrapper( std::auto_ptr<T> iService )
  : service_(iService)
  { }

  // use compiler-generated (virtual!) d'tor

  // accessor:
  T &
    get() const { return *service_; }

private:
  std::auto_ptr<T> service_;

};  // ServiceWrapper<>

// ======================================================================

#endif  // ServiceRegistry_ServiceWrapper_h
