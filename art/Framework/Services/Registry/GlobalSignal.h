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

#include "sigc++/signal.h"

#include <functional>

namespace art {
  template <detail::SignalResponseType, typename ResultType, typename... Args > class GlobalSignal;

  // Forward declaration for friendship.
  class EventProcessor;
}

template <art::detail::SignalResponseType STYPE, typename ResultType, typename... Args>
class art::GlobalSignal {
private:
  typedef sigc::signal<ResultType, Args...> SigType_;
public:
  // Typedefs
  typedef typename SigType_::slot_type slot_type;
  typedef ResultType result_type;

  // 1. Free function or functor (or pre-bound member function).
  void
  watch(std::function < ResultType(Args...) > slot);
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

  ResultType invoke(Args... args) const;
  void clear();

private:
  friend class art::EventProcessor; // SignalSentry needs signal_;

  SigType_ signal_;
};

// 1.
template <art::detail::SignalResponseType STYPE, typename ResultType, typename... Args>
void
art::GlobalSignal<STYPE, ResultType, Args...>::
watch(std::function<ResultType(Args...)> slot)
{
  detail::connect_to_signal<STYPE>(signal_, slot);
}

// 2a.
template <art::detail::SignalResponseType STYPE, typename ResultType, typename... Args>
template <typename T>
void
art::GlobalSignal<STYPE, ResultType, Args...>::
watch(ResultType(T::*slot)(Args...), T & t)
{
  watch(detail::makeWatchFunc(slot, t));
}

// 2b.
template <art::detail::SignalResponseType STYPE, typename ResultType, typename... Args>
template <typename T>
void
art::GlobalSignal<STYPE, ResultType, Args...>::
watch(T * t, ResultType(T::*slot)(Args...))
{
  watch(detail::makeWatchFunc(slot, *t));
}

// 3a.
template <art::detail::SignalResponseType STYPE, typename ResultType, typename... Args>
template <typename T>
void
art::GlobalSignal<STYPE, ResultType, Args...>::
watch(ResultType(T::*slot)(Args...) const, T const & t)
{
  watch(detail::makeWatchFunc(slot, t));
}

// 3b.
template <art::detail::SignalResponseType STYPE, typename ResultType, typename... Args>
template <typename T>
void
art::GlobalSignal<STYPE, ResultType, Args...>::
watch(T const * t, ResultType(T::*slot)(Args...) const)
{
  watch(detail::makeWatchFunc(slot, *t));
}

template <art::detail::SignalResponseType STYPE, typename ResultType, typename... Args>
ResultType
art::GlobalSignal<STYPE, ResultType, Args...>::
invoke(Args... args) const
{
  return signal_(std::forward<Args>(args)...);
}

template <art::detail::SignalResponseType STYPE, typename ResultType, typename... Args>
void
art::GlobalSignal<STYPE, ResultType, Args...>::
clear()
{
  signal_.clear();
}
#endif /* art_Framework_Services_Registry_GlobalSignal_h */

// Local Variables:
// mode: c++
// End:
