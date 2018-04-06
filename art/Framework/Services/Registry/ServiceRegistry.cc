#include "art/Framework/Services/Registry/ServiceRegistry.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Utilities/PluginSuffixes.h"
#include "boost/thread/tss.hpp"
#include "cetlib/LibraryManager.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <utility>
#include <vector>

using namespace std;
using fhicl::ParameterSet;

namespace art {

  ServiceRegistry::~ServiceRegistry() noexcept { manager_ = nullptr; }

  ServiceRegistry::ServiceRegistry() noexcept = default;

  ServiceRegistry&
  ServiceRegistry::instance() noexcept
  {
    static ServiceRegistry me;
    return me;
  }

  void
  ServiceRegistry::setManager(ServicesManager* mgr) noexcept
  {
    manager_ = mgr;
  }

} // namespace art
