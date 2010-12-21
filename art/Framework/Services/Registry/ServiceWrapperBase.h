#ifndef ServiceRegistry_ServiceWrapperBase_h
#define ServiceRegistry_ServiceWrapperBase_h

// ======================================================================
//
// ServiceWrapperBase - Base class through which the framework manages
//                      the lifetime of ServiceWrapper<T> objects.
//
// ======================================================================

namespace art { namespace serviceregistry {
  class ServiceWrapperBase;
} }

// ----------------------------------------------------------------------

class art::serviceregistry::ServiceWrapperBase
{
  // non-copyable:
  ServiceWrapperBase( ServiceWrapperBase const & );
  void operator = ( ServiceWrapperBase const & );

public:
  ServiceWrapperBase( ) { }

  virtual ~ServiceWrapperBase( );

};  // ServiceWrapperBase

// ======================================================================

#endif  // ServiceRegistry_ServiceWrapperBase_h
