#ifndef art_Framework_Core_RPManager_h
#define art_Framework_Core_RPManager_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ResultsProducer.h"
#include "art/Framework/Principal/RPWorker.h"
#include "cetlib/BasicPluginFactory.h"
#include "fhiclcpp/ParameterSet.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace art {

  class RPManager {

  public:
    template <typename RET, typename... ARGS>
    using invoke_function_t = RET (ResultsProducer::*)(ARGS...);

    using on_rpworker_t = std::function<void(RPWorker&)>;

  public:
    RPManager(fhicl::ParameterSet const& ps);

  public:
    template <typename... ARGS>
    void invoke(invoke_function_t<void, ARGS...> mfunc, ARGS&&... args);

    void for_each_RPWorker(on_rpworker_t wfunc);

  private:
    // Maps path name to RPWorkers on that path.
    std::map<std::string, std::vector<std::unique_ptr<RPWorker>>> rpmap_;
  };

  template <typename... ARGS>
  void
  RPManager::invoke(invoke_function_t<void, ARGS...> mfunc, ARGS&&... args)
  {
    for (auto& path : rpmap_) {
      for (auto& w : path.second) {
        (w->rp().*mfunc)(std::forward<ARGS>(args)...);
      }
    }
  }

} // namespace art

#endif /* art_Framework_Core_RPManager_h */

// Local Variables:
// mode: c++
// End:
