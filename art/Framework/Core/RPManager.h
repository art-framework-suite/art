#ifndef art_Framework_Core_RPManager_h
#define art_Framework_Core_RPManager_h
////////////////////////////////////////////////////////////////////////
// RPManager
//
// Art-internal class to handle the creation and management of
// ResultsProducers.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/RPWorkerT.h"
#include "cetlib/BasicPluginFactory.h"
#include "fhiclcpp/ParameterSet.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace art {
  class RPManager;
}

class art::RPManager {
public:
  using RPPath_t = std::vector<std::unique_ptr<art::RPWorker>>;
  using RPMap_t = std::map<std::string, RPPath_t>;

  template <typename RET, typename... ARGS>
  using invoke_function_t = RET (art::ResultsProducer::*)(ARGS...);

  using on_rpworker_t = std::function<void(art::RPWorker&)>;

  RPManager(fhicl::ParameterSet const& ps);

  std::size_t size() const;
  bool empty() const;

  template <typename... ARGS>
  void invoke(invoke_function_t<void, ARGS...> mfunc, ARGS&&... args);

  void for_each_RPWorker(on_rpworker_t wfunc);

private:
  cet::BasicPluginFactory pf_;
  RPMap_t rpmap_;
  std::size_t size_;

  RPMap_t makeRPs_(fhicl::ParameterSet const& ps);
};

inline std::size_t
art::RPManager::size() const
{
  return size_;
}

inline bool
art::RPManager::empty() const
{
  return size_ == 0ul;
}

template <typename... ARGS>
void
art::RPManager::invoke(invoke_function_t<void, ARGS...> mfunc, ARGS&&... args)
{
  for (auto& path : rpmap_) {
    for (auto& w : path.second) {
      (w->rp().*mfunc)(std::forward<ARGS>(args)...);
    }
  }
}

void
art::RPManager::for_each_RPWorker(on_rpworker_t wfunc)
{
  for (auto& path : rpmap_) {
    for (auto& w : path.second) {
      wfunc(*w);
    }
  }
}

#endif /* art_Framework_Core_RPManager_h */

// Local Variables:
// mode: c++
// End:
