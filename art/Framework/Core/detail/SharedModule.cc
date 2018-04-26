#include "art/Framework/Core/detail/SharedModule.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "cetlib_except/demangle.h"

#include <iomanip>

namespace art {
  namespace detail {

    SharedModule::~SharedModule() noexcept
    {
      delete chain_.load();
      chain_ = nullptr;
    }

    SharedModule::SharedModule() { chain_ = nullptr; }

    hep::concurrency::SerialTaskQueueChain*
    SharedModule::serialTaskQueueChain() const
    {
      return chain_.load();
    }

    void
    SharedModule::serialize_for_resource()
    {
      // This is the situation where a shared module must be
      // serialized, but only wrt. itself--only one event call at a
      // time.  We cannot use the module label because it has not been
      // filled in yet; so we instead use the address of any instance.
      // This is safe for a shared module, since there is guaranteed
      // to be only one instance per module label.
      std::ostringstream oss;
      oss << cet::demangle_symbol(typeid(*this).name()) << ':' << std::hex
          << this;
      serialize_for_resource(oss.str());
    }

    void
    SharedModule::serialize_for_resource(std::string const& resourceName)
    {
      auto result = resourceNames_.emplace(resourceName);
      if (result.second) {
        SharedResourcesRegistry::instance()->registerSharedResource(
          resourceName);
      }
    }

  } // namespace detail
} // namespace art
