#ifndef art_Framework_Services_Registry_ServiceHandle_h
#define art_Framework_Services_Registry_ServiceHandle_h

////////////////////////////////////////////////////////////////////////
// ServiceHandle
//
// Smart pointer used to give easy access to Services.
//
// Note invocation only requires one template argument, but the
// constructor will require zero or one arguments depending on the scope
// of the service (LEGACY, GLOBAL or PER_SCHEDULE).
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Utilities/ScheduleID.h"

namespace art {
  template< class T, ServiceScope SCOPE = art::detail::ServiceHelper<T>::scope_val> class ServiceHandle;
  template <class T> class ServiceHandle<T, art::ServiceScope::PER_SCHEDULE>;
}

// General template.
template< class T, art::ServiceScope SCOPE>
class art::ServiceHandle {
public:
  // c'tor:
  ServiceHandle()
    : instance(& ServiceRegistry::instance().template get<T>())
  { }

  // accessors:
  T  * operator -> () const  { return  instance; }
  T  & operator * () const  { return *instance; }
  T  * get() const { return instance; }

private:
  T  * instance;
};

// Per-schedule template. SFINAE wouldn't work here.
template< class T>
class art::ServiceHandle<T, art::ServiceScope::PER_SCHEDULE> {
public:
  // c'tor:
  ServiceHandle(ScheduleID sID)
    : instance(& ServiceRegistry::instance().template get<T>(sID))
  { }

  // accessors:
  T  * operator -> () const  { return  instance; }
  T  & operator * () const  { return *instance; }
  T  * get() const { return instance; }

private:
  T  * instance;
};

// ======================================================================

#endif /* art_Framework_Services_Registry_ServiceHandle_h */

// Local Variables:
// mode: c++
// End:
