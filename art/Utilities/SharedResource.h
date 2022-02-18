#ifndef art_Utilities_SharedResource_h
#define art_Utilities_SharedResource_h

#include "hep_concurrency/SerialTaskQueue.h"

#include <map>
#include <set>
#include <string>
#include <typeinfo>
#include <vector>

#include <tbb/task_group.h> // Can't forward-declare this class.

namespace art {
  namespace detail {
    struct SharedResource_t {
      SharedResource_t(std::string const& name, bool demangle);
      ~SharedResource_t();
      std::string name;
    };
    extern SharedResource_t const LegacyResource;
  }

  template <typename T>
  detail::SharedResource_t SharedResource{typeid(T).name(), true};
}

// =====================================================================

namespace art::detail {
  class SharedResources {
  public:
    SharedResources();

    void registerSharedResources(std::set<std::string> const& names);
    void registerSharedResource(detail::SharedResource_t const&);
    void freeze(tbb::task_group& group);

    using queue_ptr_t = std::shared_ptr<hep::concurrency::SerialTaskQueue>;
    std::vector<queue_ptr_t> createQueues(
      std::vector<std::string> const& resourceNames) const;

  private:
    void register_resource(std::string const& name);
    void ensure_not_frozen(std::string const& name);

    std::map<std::string, unsigned> resourceCounts_;
    std::vector<std::pair<std::string, queue_ptr_t>> sortedResources_;
    bool frozen_{false};
    unsigned nLegacy_{};
  };
}

#endif /* art_Utilities_SharedResource_h */

// Local Variables:
// mode: c++
// End:
