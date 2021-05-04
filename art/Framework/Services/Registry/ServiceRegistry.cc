#include "art/Framework/Services/Registry/ServiceRegistry.h"
// vim: set sw=2 expandtab :

namespace art {

  ServiceRegistry::~ServiceRegistry() noexcept = default;
  ServiceRegistry::ServiceRegistry() noexcept = default;

  ServiceRegistry&
  ServiceRegistry::instance() noexcept
  {
    static ServiceRegistry me;
    return me;
  }

  void
  ServiceRegistry::setManager(ServicesManager* mgr)
  {
    manager_ = mgr;
  }

} // namespace art
