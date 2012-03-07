#ifndef art_Framework_Services_Registry_ServiceWrapperBase_h
#define art_Framework_Services_Registry_ServiceWrapperBase_h

// ======================================================================
//
// ServiceWrapperBase - Base class through which the framework manages
//                      the lifetime of ServiceWrapper<T> objects.
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "fhiclcpp/ParameterSet.h"

namespace art {
  class ServiceWrapperBase;
}

// ----------------------------------------------------------------------

class art::ServiceWrapperBase {
  // non-copyable:
  ServiceWrapperBase(ServiceWrapperBase const &);
  void operator = (ServiceWrapperBase const &);

public:
  ServiceWrapperBase() { }

  virtual ~ServiceWrapperBase();
  void reconfigure(fhicl::ParameterSet const &);
  ServiceScope scope() const;

private:
  virtual void reconfigure_service(fhicl::ParameterSet const &) = 0;
  virtual ServiceScope service_scope() const = 0;
};  // ServiceWrapperBase

inline
void
art::ServiceWrapperBase::reconfigure(fhicl::ParameterSet const & pset)
{
  reconfigure_service(pset);
}

inline
art::ServiceScope
art::ServiceWrapperBase::scope() const
{
  return service_scope();
}
// ======================================================================

#endif /* art_Framework_Services_Registry_ServiceWrapperBase_h */

// Local Variables:
// mode: c++
// End:
