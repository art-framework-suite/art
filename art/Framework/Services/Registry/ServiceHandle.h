#ifndef art_Framework_Services_Registry_ServiceHandle_h
#define art_Framework_Services_Registry_ServiceHandle_h

////////////////////////////////////////////////////////////////////////
// ServiceHandle
//
// Smart pointer used to give easy access to Services.
//
// Note invocation only requires one template argument, that of the
// service type to which access is required.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Persistency/Provenance/ExecutionContext.h"
#include "art/Persistency/Provenance/ExecutionContextManager.h"

namespace art {
  template <typename T,
            ServiceScope SCOPE = art::detail::ServiceHelper<T>::scope_val>
  class ServiceHandle;
  template <class T> class ServiceHandle<T, art::ServiceScope::LOCAL>;
}

// General template.
template <typename T, art::ServiceScope SCOPE>
class art::ServiceHandle {
public:
  ServiceHandle();

  T  * operator -> () const;
  T  & operator * () const;

private:
  T  * service_;
};

template <typename T, art::ServiceScope SCOPE>
inline
art::ServiceHandle<T, SCOPE>::
ServiceHandle()
:
  service_(&ServiceRegistry::instance().template get<T>())
{
}

template <typename T, art::ServiceScope SCOPE>
inline
auto
art::ServiceHandle<T, SCOPE>::
operator -> () const
-> T *
{
  return service_;
}

template <typename T, art::ServiceScope SCOPE>
inline
auto
art::ServiceHandle<T, SCOPE>::
operator * () const
-> T &
{
  return *service_;
}

// Specialization for local services.
template <class T>
class art::ServiceHandle<T, art::ServiceScope::LOCAL> {
public:
  ServiceHandle();

  T  * operator -> () const;
  T  & operator * () const;

  void swap(ServiceHandle & other);

private:
  T  * service_;
  ExecutionContext context_;
};

template <typename T>
inline
art::ServiceHandle<T, art::ServiceScope::LOCAL>::
ServiceHandle()
:
  service_(&ServiceRegistry::instance().template get<T>()),
  context_(ExecutionContextManager::top())
{
}

template <typename T>
inline
auto
art::ServiceHandle<T, art::ServiceScope::LOCAL>::
operator -> () const
-> T *
{
  if (!compareContexts(env_, ExecutionContextManager::top())) {
    LogWarn("LocalServiceContext")
      << "ServiceHandle no longer valid in this context.\n"
      << "Obtaining context-appropriate local service.\n"
      << "To avoid this message (and the performance penalty of "
      << "re-initializing\nthis ServiceHandle), do not cache the "
      << "ServiceHandle at module-class scope.";
    ServiceHandle<T> tmp; // Get current context.
    swap(tmp);
  }
  return service_;
}

template <typename T>
inline
auto
art::ServiceHandle<T, art::ServiceScope::LOCAL>::
operator * () const
-> T &
{
  return *(this->operator &());
}

template <typename T>
inline
void
art::ServiceHandle<T, art::ServiceScope::LOCAL>::
swap(ServiceHandle & other)
{
  using std::swap;
  swap(service_);
  swap(context_);
}

#endif /* art_Framework_Services_Registry_ServiceHandle_h */

// Local Variables:
// mode: c++
// End:
