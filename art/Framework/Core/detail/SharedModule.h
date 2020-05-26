#ifndef art_Framework_Core_detail_SharedModule_h
#define art_Framework_Core_detail_SharedModule_h
// vim: set sw=2 expandtab :

#include "art/Utilities/SharedResource.h"
#include "canvas/Persistency/Provenance/BranchType.h"

#include <atomic>
#include <set>
#include <string>

namespace hep::concurrency {
  class SerialTaskQueueChain;
}

namespace art::detail {

  class SharedModule {
  public:
    SharedModule();
    explicit SharedModule(std::string const& moduleLabel);
    ~SharedModule() noexcept;

    hep::concurrency::SerialTaskQueueChain* serialTaskQueueChain() const;

    void createQueues();

    template <BranchType BT = InEvent, typename... T>
    void serialize(T const&...);

    template <BranchType BT = InEvent, typename... T>
    void serializeExternal(T const&...);

    template <BranchType BT = InEvent>
    void
    async()
    {
      static_assert(
        BT == InEvent,
        "async is currently supported only for the 'InEvent' level.");
      asyncDeclared_ = true;
    }

  private:
    void implicit_serialize();
    void serialize_for(detail::SharedResource_t const&);
    void serialize_for_external(std::string const&);

    template <typename... T>
    void
    serialize_for_resource(T const&... t)
    {
      static_assert(
        std::conjunction_v<std::is_same<detail::SharedResource_t, T>...>);
      if (sizeof...(t) == 0) {
        implicit_serialize();
      } else {
        (serialize_for(t), ...);
      }
    }

    template <typename... T>
    void
    serialize_for_external_resource(T const&... t)
    {
      static_assert(std::conjunction_v<std::is_same<std::string, T>...>);
      if (sizeof...(t) == 0) {
        implicit_serialize();
      } else {
        (serialize_for_external(t), ...);
      }
    }

    std::string moduleLabel_;
    std::set<std::string> resourceNames_{};
    bool asyncDeclared_{false};
    std::atomic<hep::concurrency::SerialTaskQueueChain*> chain_;
  };

  template <BranchType, typename... T>
  void
  SharedModule::serialize(T const&... resources)
  {
    serialize_for_resource(resources...);
  }

  template <BranchType, typename... T>
  void
  SharedModule::serializeExternal(T const&... resources)
  {
    serialize_for_external_resource(resources...);
  }
}

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_detail_SharedModule_h */
