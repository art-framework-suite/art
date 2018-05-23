#ifndef art_Framework_Core_detail_SharedModule_h
#define art_Framework_Core_detail_SharedModule_h
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchType.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <atomic>
#include <set>
#include <string>

namespace art {
  namespace detail {

    class SharedModule {
    public:
      SharedModule();
      explicit SharedModule(std::string const& moduleLabel);
      ~SharedModule() noexcept;

      hep::concurrency::SerialTaskQueueChain* serialTaskQueueChain() const;

      void createQueues();

      template <BranchType BT = InEvent, typename... T>
      void serialize(T const&...);

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
      void serialize_for_resource();
      void serialize_for_resource(std::string const&);

      template <typename H, typename... T>
      std::enable_if_t<std::is_same<std::string, H>::value>
      serialize_for_resource(H const& head, T const&... tail)
      {
        serialize_for_resource(head);
        if (sizeof...(tail) != 0) {
          serialize_for_resource(tail...);
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
  }
}

  // Local Variables:
  // mode: c++
  // End:

#endif /* art_Framework_Core_detail_SharedModule_h */
