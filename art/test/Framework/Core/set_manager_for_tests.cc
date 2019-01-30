#include "art/Framework/Services/Registry/ServiceRegistry.h"

void
art::test::set_manager_for_tests(ServicesManager* const manager)
{
  ServiceRegistry::instance().setManager(manager);
}
