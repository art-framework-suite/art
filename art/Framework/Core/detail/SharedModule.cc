#include "art/Framework/Core/detail/SharedModule.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"

#include <string>
#include <vector>

using namespace std::string_literals;
using namespace hep::concurrency;

namespace art {
  namespace detail {

    SharedModule::~SharedModule() noexcept
    {
      delete chain_.load();
      chain_ = nullptr;
    }

    SharedModule::SharedModule() : SharedModule{""s} {}

    SharedModule::SharedModule(std::string const& moduleLabel)
      : moduleLabel_{moduleLabel}
    {
      chain_ = nullptr;
    }

    SerialTaskQueueChain*
    SharedModule::serialTaskQueueChain() const
    {
      return chain_.load();
    }

    void
    SharedModule::createQueues()
    {
      Exception e{errors::LogicError,
                  "An error occurred while processing scheduling options for a "
                  "module.\n"};
      if (asyncDeclared_) {
        if (resourceNames_.empty()) {
          return;
        }
        throw e
          << "async<art::InEvent>() cannot be called in combination with any "
             "serialize<art::InEvent>(...) calls.\n";
      }

      if (resourceNames_.empty()) {
        throw e << "Either 'async<art::InEvent>()' or "
                   "'serialize<art::InEvent>(...)'\n"
                   "must be called for a shared module.\n";
      }
      std::vector<std::string> const names(cbegin(resourceNames_),
                                           cend(resourceNames_));
      auto queues = SharedResourcesRegistry::instance()->createQueues(names);
      chain_ = new SerialTaskQueueChain{queues};
    }

    void
    SharedModule::serialize_for_resource()
    {
      // This is the situation where a shared module must be
      // serialized, but only wrt. itself--only one event call at a
      // time.
      serialize_for_resource(moduleLabel_);
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
