#include "art/Framework/Core/detail/SharedModule.h"
// vim: set sw=2 expandtab :

#include "art/Utilities/SharedResource.h"
#include "canvas/Utilities/Exception.h"

#include <string>
#include <vector>

using namespace hep::concurrency;

namespace art::detail {

  SharedModule::SharedModule() = default;
  SharedModule::~SharedModule() = default;

  SharedModule::SharedModule(std::string const& moduleLabel)
    : moduleLabel_{moduleLabel}
  {}

  SerialTaskQueueChain*
  SharedModule::serialTaskQueueChain() const
  {
    return chain_.get();
  }

  std::set<std::string> const&
  SharedModule::sharedResources() const
  {
    return resourceNames_;
  }

  void
  SharedModule::createQueues(SharedResources const& resources)
  {
    Exception e{errors::LogicError,
                "An error occurred while processing scheduling options for a "
                "module.\n"};
    if (asyncDeclared_) {
      if (empty(resourceNames_)) {
        return;
      }
      throw e
        << "async<art::InEvent>() cannot be called in combination with any "
           "serialize<art::InEvent>(...) calls.\n";
    }

    if (empty(resourceNames_)) {
      throw e << "Either 'async<art::InEvent>()' or "
                 "'serialize<art::InEvent>(...)'\n"
                 "must be called in a shared module's constructor.\n";
    }
    std::vector<std::string> const names(cbegin(resourceNames_),
                                         cend(resourceNames_));
    auto queues = resources.createQueues(names);
    chain_ = std::make_unique<SerialTaskQueueChain>(queues);
  }

  void
  SharedModule::implicit_serialize()
  {
    // This is the situation where a shared module must be serialized,
    // but only wrt. itself--i.e. only one event call at a time.
    // Because the shared-resources registry is not prepopulated with
    // module names, we call the 'serialize_for_external' function to
    // insert an entry into the registry.  This is safe to do as only
    // this module will refer to the shared resource.
    serialize_for(moduleLabel_);
  }

  void
  SharedModule::serialize_for(std::string const& resourceName)
  {
    resourceNames_.emplace(resourceName);
  }
}
