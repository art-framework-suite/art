#ifndef art_Framework_Services_Registry_detail_makeWatchFunc_h
#define art_Framework_Services_Registry_detail_makeWatchFunc_h

#include <functional>
#include <type_traits>

////////////////////////////////////////////////////////////////////////
// makeWatchFunc
//
// Construct the correct lambda to allow general registration of
// pointer-to-member callback functions for global and local signals.
namespace art {
  namespace detail {
    template <typename T, typename ResultType, typename...Args>
    std::function<ResultType(Args...)>
    makeWatchFunc(ResultType(T::*slot)(Args...), T & t)
    {
      return [slot, &t](Args && ... args) -> ResultType
      { return (t.*slot)(std::forward<Args>(args)...); };
    }

    template <typename T, typename ResultType, typename...Args>
    std::function<ResultType(Args...)>
    makeWatchFunc(ResultType(T::*slot)(Args...) const, T const & t)
    {
      return [slot, &t](Args && ... args) -> ResultType
      { return (t.*slot)(std::forward<Args>(args)...); };
    }
  }
}
#endif /* art_Framework_Services_Registry_detail_makeWatchFunc_h */

// Local Variables:
// mode: c++
// End:
