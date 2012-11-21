#ifndef art_Framework_Services_Registry_GlobalSignal_h
#define art_Framework_Services_Registry_GlobalSignal_h

// Define a wrapper for global signals. The invoke() and clear()
// functions are intended to be called only by art code.

#include "art/Framework/Services/Registry/detail/SignalResponseType.h"
#include "cpp0x/functional"
#include "sigc++/signal.h"

namespace art {
  template <detail::SignalResponseType, typename ResultType, typename... Args > class GlobalSignal;
}

template <art::detail::SignalResponseType STYPE, typename ResultType, typename... Args>
class art::GlobalSignal {
private:
  typedef sigc::signal<ResultType, Args...> SigType_;
public:
  // Typedefs
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
  watch(std::bind(slot, t));
}

// 2b.
template <art::detail::SignalResponseType STYPE, typename ResultType, typename... Args>
template <typename T>
void
art::GlobalSignal<STYPE, ResultType, Args...>::
watch(T * t, ResultType(T::*slot)(Args...))
{
  watch(std::bind(slot, t));
}

// 3a.
template <art::detail::SignalResponseType STYPE, typename ResultType, typename... Args>
template <typename T>
void
art::GlobalSignal<STYPE, ResultType, Args...>::
watch(ResultType(T::*slot)(Args...) const, T const & t)
{
  watch(std::bind(slot, t));
}

// 3b.
template <art::detail::SignalResponseType STYPE, typename ResultType, typename... Args>
template <typename T>
void
art::GlobalSignal<STYPE, ResultType, Args...>::
watch(T const * t, ResultType(T::*slot)(Args...) const)
{
  watch(std::bind(slot, t));
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
