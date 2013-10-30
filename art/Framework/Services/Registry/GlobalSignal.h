#ifndef art_Framework_Services_Registry_GlobalSignal_h
#define art_Framework_Services_Registry_GlobalSignal_h

////////////////////////////////////////////////////////////////////////
// GlobalSignal.h
//
// Define a wrapper for global signals. The watch(...) functions are for
// users wishing to register for callbacks; the invoke() and clear()
// functions are intended to be called only by art code.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/detail/SignalResponseType.h"
#include "art/Framework/Services/Registry/detail/makeWatchFunc.h"

#include <deque>
#include <functional>

namespace art {
  template <detail::SignalResponseType, typename ResultType, typename... Args > class GlobalSignal;
}

template <art::detail::SignalResponseType SRTYPE, typename ResultType, typename... Args>
class art::GlobalSignal {
public:
  // Typedefs
  typedef std::function<ResultType(Args...)> slot_type;
  typedef ResultType result_type;

  // 1. Free function or functor (or pre-bound member function).
  void
  watch(std::function<ResultType(Args...)> slot);
  // 2a. Non-const member function.
  template <typename T>
  void
  watch(ResultType(T::*slot)(Args...), T & t);
  // 2b. Non-const member function (legacy).
  template <typename T>
  void
  watch(T * t, ResultType(T::*slot)(Args...));
  // 3a. Const member function.
  template <typename T>
  void
  watch(ResultType(T::*slot)(Args...) const, T const & t);
  // 3b. Const member function (legacy).
  template <typename T>
  void
  watch(T const * t, ResultType(T::*slot)(Args...) const);

  void invoke(Args && ... args) const; // Discard ResultType.

  void clear();

private:
  std::deque<slot_type> signal_;
};

// 1.
template <art::detail::SignalResponseType SRTYPE, typename ResultType, typename... Args>
void
art::GlobalSignal<SRTYPE, ResultType, Args...>::
watch(std::function<ResultType(Args...)> slot)
{
  detail::connect_to_signal<SRTYPE>(signal_, slot);
}

// 2a.
template <art::detail::SignalResponseType SRTYPE, typename ResultType, typename... Args>
template <typename T>
void
art::GlobalSignal<SRTYPE, ResultType, Args...>::
watch(ResultType(T::*slot)(Args...), T & t)
{
  watch(detail::makeWatchFunc(slot, t));
}

// 2b.
template <art::detail::SignalResponseType SRTYPE, typename ResultType, typename... Args>
template <typename T>
void
art::GlobalSignal<SRTYPE, ResultType, Args...>::
watch(T * t, ResultType(T::*slot)(Args...))
{
  watch(detail::makeWatchFunc(slot, *t));
}

// 3a.
template <art::detail::SignalResponseType SRTYPE, typename ResultType, typename... Args>
template <typename T>
void
art::GlobalSignal<SRTYPE, ResultType, Args...>::
watch(ResultType(T::*slot)(Args...) const, T const & t)
{
  watch(detail::makeWatchFunc(slot, t));
}

// 3b.
template <art::detail::SignalResponseType SRTYPE, typename ResultType, typename... Args>
template <typename T>
void
art::GlobalSignal<SRTYPE, ResultType, Args...>::
watch(T const * t, ResultType(T::*slot)(Args...) const)
{
  watch(detail::makeWatchFunc(slot, *t));
}

template <art::detail::SignalResponseType SRTYPE, typename ResultType, typename... Args>
void
art::GlobalSignal<SRTYPE, ResultType, Args...>::
invoke(Args && ... args) const
{
  for (auto f : signal_) {
    f(std::forward<Args>(args)...);
  }
}

template <art::detail::SignalResponseType SRTYPE, typename ResultType, typename... Args>
void
art::GlobalSignal<SRTYPE, ResultType, Args...>::
clear()
{
  signal_.clear();
}
#endif /* art_Framework_Services_Registry_GlobalSignal_h */

// Local Variables:
// mode: c++
// End:
