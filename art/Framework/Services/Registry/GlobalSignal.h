#ifndef art_Framework_Services_Registry_GlobalSignal_h
#define art_Framework_Services_Registry_GlobalSignal_h

// Define a wrapper for global signals. The invoke() and clear() signals
// are intended to be called only be art::ActivityRegistry.

#include "art/Framework/Services/Registry/detail/SignalResponseType.h"
#include "cpp0x/functional"
#include "sigc++/signal.h"

namespace art {
  template <detail::SignalResponseType, typename ReturnType, typename... Args > class GlobalSignal;
}

template <art::detail::SignalResponseType STYPE, typename ReturnType, typename... Args>
class art::GlobalSignal {
private:
  typedef sigc::signal<ReturnType, Args...> SigType_;
public:
  // 1. Free function or functor (or pre-bound member function).
  void
  watch(std::function < ReturnType(Args...) > slot);
  // 2. Non-const member function.
  template <typename T>
  void
  watch(ReturnType(T::*slot)(Args...), T & t);
  // 3. Const member function.
  template <typename T>
  void
  watch(ReturnType(T::*slot)(Args...) const, T const & t);

  ReturnType invoke(Args... args) const;
  void clear();

private:
  SigType_ signal_;
};

// 1.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename... Args>
void
art::GlobalSignal<STYPE, ReturnType, Args...>::
watch(std::function<ReturnType(Args...)> slot)
{
  detail::connect_to_signal<STYPE>(signal_, slot);
}

// 2.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename... Args>
template <typename T>
void
art::GlobalSignal<STYPE, ReturnType, Args...>::
watch(ReturnType(T::*slot)(Args...), T & t)
{
  watch(std::bind(slot, t));
}

// 3.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename... Args>
template <typename T>
void
art::GlobalSignal<STYPE, ReturnType, Args...>::
watch(ReturnType(T::*slot)(Args...) const, T const & t)
{
  watch(std::bind(slot, t));
}

template <art::detail::SignalResponseType STYPE, typename ReturnType, typename... Args>
ReturnType
art::GlobalSignal<STYPE, ReturnType, Args...>::
invoke(Args... args) const
{
  return signal_(std::forward<Args>(args)...);
}

template <art::detail::SignalResponseType STYPE, typename ReturnType, typename... Args>
void
art::GlobalSignal<STYPE, ReturnType, Args...>::
clear()
{
  signal_.clear();
}
#endif /* art_Framework_Services_Registry_GlobalSignal_h */

// Local Variables:
// mode: c++
// End:
