#ifndef ServiceRegistry_ServiceWrapper_h
#define ServiceRegistry_ServiceWrapper_h

// ======================================================================
//
// ServiceWrapper - Class template through which the framework manages
//                  the lifetime of a service.
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceWrapperBase.h"
#include <memory>

namespace art {
  template< class T >
    class ServiceWrapper;
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

private:
  std::auto_ptr<T> service_ptr_;

};  // ServiceWrapper<>

// ======================================================================

#endif
