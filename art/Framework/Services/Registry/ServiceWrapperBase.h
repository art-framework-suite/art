#ifndef art_Framework_Services_Registry_ServiceWrapperBase_h
#define art_Framework_Services_Registry_ServiceWrapperBase_h

// ======================================================================
//
// ServiceWrapperBase - Base class through which the framework manages
//                      the lifetime of ServiceWrapper<T> objects.
//
// ======================================================================

#include "fhiclcpp/ParameterSet.h"

namespace art {
  class ServiceWrapperBase;
}

// ----------------------------------------------------------------------

class art::ServiceWrapperBase
{
  // non-copyable:
  ServiceWrapperBase( ServiceWrapperBase const & );
  void operator = ( ServiceWrapperBase const & );

public:
  ServiceWrapperBase( ) { }

  virtual ~ServiceWrapperBase( );
  virtual void reconfigure(fhicl::ParameterSet const&);

};  // ServiceWrapperBase

// ======================================================================

#endif /* art_Framework_Services_Registry_ServiceWrapperBase_h */

// Local Variables:
// mode: c++
// End:
