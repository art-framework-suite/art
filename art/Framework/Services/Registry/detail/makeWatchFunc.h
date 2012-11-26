#ifndef art_Framework_Services_Registry_detail_makeWatchFunc_h
#define art_Framework_Services_Registry_detail_makeWatchFunc_h

#include <functional>
#include <type_traits>

////////////////////////////////////////////////////////////////////////
// makeWatchFunc
//
// Overloaded SFINAE functions to ensure the correct binding gets done
// in GlobalSignal and LocalSignal..
namespace art {
  namespace detail {
    template <typename T, typename ResultType, typename...Args>
    typename std::enable_if<sizeof...(Args) == 0, std::function<ResultType(Args...)>>::type
    makeWatchFunc(ResultType(T::*slot)(Args...), T & t) {
      return std::bind(slot, std::ref(t));
    }
    template <typename T, typename ResultType, typename...Args>
    typename std::enable_if<sizeof...(Args) == 1, std::function<ResultType(Args...)>>::type
    makeWatchFunc(ResultType(T::*slot)(Args...), T & t) {
      return std::bind(slot, std::ref(t), std::placeholders::_1);
    }
    template <typename T, typename ResultType, typename...Args>
    typename std::enable_if<sizeof...(Args) == 2, std::function<ResultType(Args...)>>::type
    makeWatchFunc(ResultType(T::*slot)(Args...), T & t) {
      return std::bind(slot, std::ref(t), std::placeholders::_1, std::placeholders::_2);
    }

    template <typename T, typename ResultType, typename...Args>
    typename std::enable_if<sizeof...(Args) == 0, std::function<ResultType(Args...)>>::type
    makeWatchFunc(ResultType(T::*slot)(Args...) const, T const & t) {
      return std::bind(slot, std::cref(t));
    }
    template <typename T, typename ResultType, typename...Args>
    typename std::enable_if<sizeof...(Args) == 1, std::function<ResultType(Args...)>>::type
    makeWatchFunc(ResultType(T::*slot)(Args...) const, T const & t) {
      return std::bind(slot, std::cref(t), std::placeholders::_1);
    }
    template <typename T, typename ResultType, typename...Args>
    typename std::enable_if<sizeof...(Args) == 2, std::function<ResultType(Args...)>>::type
    makeWatchFunc(ResultType(T::*slot)(Args...) const, T const & t) {
      return std::bind(slot, std::cref(t), std::placeholders::_1, std::placeholders::_2);
    }
  }
}
#endif /* art_Framework_Services_Registry_detail_makeWatchFunc_h */

// Local Variables:
// mode: c++
// End:
