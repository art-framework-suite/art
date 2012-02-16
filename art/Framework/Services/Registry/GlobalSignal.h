#ifndef art_Framework_Services_Registry_GlobalSignal_h
#define art_Framework_Services_Registry_GlobalSignal_h

// Define a wrapper for global signals. The invoke() and clear() signals
// are intended to be called only be art::ActivityRegistry.

#include "art/Framework/Services/Registry/detail/SignalResponseType.h"
#include "cpp0x/functional"
#include "sigc++/signal.h"

#if ! ( defined (ART_NO_FIXED_VARIADIC_EXPANSION) || (GCC_IS_AT_LEAST(4, 7, 0)) )
#define ART_NO_FIXED_VARIADIC_EXPANSION
#endif

namespace art {
#ifdef ART_NO_FIXED_VARIADIC_EXPANSION
  template <detail::SignalResponseType STYPE, typename ReturnType, typename A1 = void, typename A2 = void> class GlobalSignal;
  template <detail::SignalResponseType STYPE, typename ReturnType, typename A1> class GlobalSignal<STYPE, ReturnType, A1>;
  template <detail::SignalResponseType STYPE, typename ReturnType> class GlobalSignal<STYPE, ReturnType>;
#else
  template <detail::SignalResponseType, typename ReturnType, typename... Args > class GlobalSignal;
#endif
}

#ifdef ART_NO_FIXED_VARIADIC_EXPANSION
// Zero argument callbacks.
template <art::detail::SignalResponseType STYPE, typename ReturnType>
class art::GlobalSignal<STYPE, ReturnType> {
private:
  typedef sigc::signal<ReturnType> SigType_;
public:
  // 1. Free function or functor (or pre-bound member function).
  void
  watch(std::function<ReturnType()> slot) {
    detail::connect_to_signal<STYPE>(signal_, slot);
  }
  // 2. Non-const member function.
  template <typename T>
  void
  watch(ReturnType(T::*slot)(), T & t) {
    watch(std::bind(slot, t));
  }
  // 3. Const member function.
  template <typename T>
  void
  watch(ReturnType(T::*slot)() const, T const & t) {
    watch(std::bind(slot, t));
  }

  ReturnType invoke() const { return signal_.emit(); }
  void clear() { signal_.clear(); }

private:
  SigType_ signal_;
};
// One argument callbacks.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename A1> class art::GlobalSignal<STYPE, ReturnType, A1> {
private:
  typedef sigc::signal<ReturnType, A1> SigType_;
public:
  // 1. Free function or functor (or pre-bound member function).
  void
  watch(std::function<ReturnType(A1)> slot) {
    detail::connect_to_signal<STYPE>(signal_, slot);
  }
  // 2. Non-const member function.
  template <typename T>
  void
  watch(ReturnType(T::*slot)(A1), T & t) {
    watch(std::bind(slot, t));
  }
  // 3. Const member function.
  template <typename T>
  void
  watch(ReturnType(T::*slot)(A1) const, T const & t) {
    watch(std::bind(slot, t));
  }

  ReturnType invoke(A1 a1) const { return signal_.emit(a1); }
  void clear() { signal_.clear(); }

private:
  SigType_ signal_;
};
// Two argument callbacks.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename A1, typename A2> class art::GlobalSignal {
private:
  typedef sigc::signal<ReturnType, A1, A2> SigType_;
public:
  // 1. Free function or functor (or pre-bound member function).
  void
  watch(std::function<ReturnType(A1, A2)> slot) {
    detail::connect_to_signal<STYPE>(signal_, slot);
  }
  // 2. Non-const member function.
  template <typename T>
  void
  watch(ReturnType(T::*slot)(A1, A2), T & t) {
    watch(std::bind(slot, t));
  }
  // 3. Const member function.
  template <typename T>
  void
  watch(ReturnType(T::*slot)(A1, A2) const, T const & t) {
    watch(std::bind(slot, t));
  }

  ReturnType invoke(A1 a1, A2 a2) const { return signal_.emit(a1, a2); }
  void clear() { signal_.clear(); }

private:
  SigType_ signal_;
};
#else
// For when variadic templates work per C++ 2011.
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
template <typename ReturnType, typename... Args>
void
art::GlobalSignal<ReturnType, Args...>::
watch(std::function<ReturnType(Args...)> slot)
{
  detail::connect_to_signal(signal_, slot);
}

// 2.
template <typename ReturnType, typename... Args>
template <typename T>
void
art::GlobalSignal<ReturnType, Args...>::
watch(ReturnType(T::*slot)(Args...), T & t)
{
  watch(std::bind(slot, t));
}

// 3.
template <typename ReturnType, typename... Args>
template <typename T>
void
art::GlobalSignal<ReturnType, Args...>::
watch(ReturnType(T::*slot)(Args...) const, T const & t)
{
  watch(std::bind(slot, t));
}

template <typename ReturnType, typename... Args>
ReturnType
art::GlobalSignal<ReturnType, Args...>::
invoke(Args... args)
{
  return signal_(std::forward<Args>(args)...);
}

template <typename ReturnType, typename... Args>
void
art::GlobalSignal<ReturnType, Args...>::
clear()
{
  signal_.clear();
}
#endif /* ART_NO_FIXED_VARIADIC_EXPANSION */

#endif /* art_Framework_Services_Registry_GlobalSignal_h */

// Local Variables:
// mode: c++
// End:
