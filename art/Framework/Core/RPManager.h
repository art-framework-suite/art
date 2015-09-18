#ifndef art_Framework_Core_RPManager_h
#define art_Framework_Core_RPManager_h

#include "art/Framework/Core/RPWrapper.h"
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
  using RPPath_t = std::vector<std::unique_ptr<art::RPWrapperBase> >;
  using RPMap_t = std::map<std::string, RPPath_t>;

  template <typename RET>
  using invoke_results_t = std::map<std::string, std::vector<RET>>;

  template <typename RET, typename... ARGS>
  using invoke_function_t = RET (art::ResultsProducer::*) (ARGS...);

  using on_wrapper_t = std::function<void (art::RPWrapperBase &)>;

  RPManager(fhicl::ParameterSet const & ps);

  std::size_t size() const;
  bool empty() const;

  template <typename ... ARGS>
  void
  invoke(invoke_function_t<void, ARGS...> mfunc,
         ARGS && ... args);

  // template <typename RET, typename ... ARGS>
  // void
  // invoke(invoke_results_t<RET> & ret,
  //        invoke_function_t<RET, ARGS...> mfunc,
  //        ARGS && ... args);

  void
  for_each_RPWrapper(on_wrapper_t wfunc);

  // No use case for these yet.
  // RPMap_t & allPaths();
  // RPMap_t const & allPaths() const;

  // RPPath_t & path(std::string const & pathName);
  // RPPath_t const & path(std::string const & pathName) const;

private:
  cet::BasicPluginFactory pf_;
  RPMap_t rpmap_;
  std::size_t size_;

  RPMap_t makeRPs_(fhicl::ParameterSet const & ps);
};

inline
std::size_t
art::RPManager::
size() const
{
  return size_;
}

inline
bool
art::RPManager::
empty() const
{
  return size_ == 0ul;
}

template <typename... ARGS>
void
art::RPManager::
invoke(invoke_function_t<void, ARGS...> mfunc,
       ARGS &&... args)
{
  for (auto & path : rpmap_) {
    for (auto & w : path.second) {
      (w->rp().*mfunc)(std::forward<ARGS>(args)...);
    }
  }
}

// template <typename RET, typename... ARGS>
// void
// art::RPManager::
// invoke(invoke_results_t<RET> & ret,
//        invoke_function_t<RET, ARGS...> mfunc,
//        ARGS &&... args)
// {
//   for (auto & path : rpmap_) {
//     for (auto & w : path.second) {
//       ret.push_back((w->rp().*mfunc)(std::forward<ARGS>(args)...));
//     }
//   }
// }

void
art::RPManager::
for_each_RPWrapper(on_wrapper_t wfunc)
{
  for (auto & path : rpmap_) {
    for (auto & w : path.second) {
      wfunc(*w);
    }
  }
}

// inline
// auto
// art::RPManager::
// allPaths()
// -> RPMap_t &
// {
//   return rpmap_;
// }

// inline
// auto
// art::RPManager::
// allPaths() const
// -> RPMap_t const &
// {
//   return rpmap_;
// }

#endif /* art_Framework_Core_RPManager_h */

// Local Variables:
// mode: c++
// End:
