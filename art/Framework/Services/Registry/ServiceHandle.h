#ifndef ServiceRegistry_ServiceHandle_h
#define ServiceRegistry_ServiceHandle_h

// ======================================================================
//
// ServiceHandle - smart pointer used to give easy access to Services.
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceRegistry.h"

namespace art {
  template< class T > class ServiceHandle;
}

// ----------------------------------------------------------------------

template< class T >
  class art::ServiceHandle
{
public:
  // c'tor:
  ServiceHandle( )
  : instance( & ServiceRegistry::instance().template get<T>() )
  { }

  // accessors:
  T *  operator -> ( ) const  { return  instance; }
  T &  operator *  ( ) const  { return *instance; }

private:
  T *  instance;

};  // ServiceHandle

// ======================================================================

#endif  // ServiceRegistry_ServiceHandle_h
