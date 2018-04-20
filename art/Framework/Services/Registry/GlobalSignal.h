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

#include "art/Framework/Principal/Consumer.h"
#include "art/Framework/Services/Registry/detail/SignalResponseType.h"
#include "art/Framework/Services/Registry/detail/makeWatchFunc.h"
#include "hep_concurrency/RecursiveMutex.h"

#include <deque>
#include <functional>

namespace art {
  template <detail::SignalResponseType, typename ResultType, typename... Args>
  class GlobalSignal;

  // Only supported template definition is a partial specialization on
  // a function type--i.e. and instantiation of GlobalSignal must look
  // like (e.g.):
  //     GlobalSignal<LIFO, void(std::string const&)> s;
  template <detail::SignalResponseType SRTYPE,
            typename ResultType,
            typename... Args>
  class GlobalSignal<SRTYPE, ResultType(Args...)> {
  public:
    // Typedefs
    using slot_type = std::function<ResultType(Args...)>;
    using result_type = ResultType;

    // 1. Free function or functor (or pre-bound member function).
    void watch(std::function<ResultType(Args...)> slot);
    // 2a. Non-const member function.
    template <typename T>
    void watch(ResultType (T::*slot)(Args...), T& t);
    // 2b. Non-const member function (legacy).
    template <typename T>
    void watch(T* t, ResultType (T::*slot)(Args...));
    // 3a. Const member function.
    template <typename T>
    void watch(ResultType (T::*slot)(Args...) const, T const& t);
    // 3b. Const member function (legacy).
    template <typename T>
    void watch(T const* t, ResultType (T::*slot)(Args...) const);

    void invoke(Args&&... args) const; // Discard ResultType.

  private:
    // Protects all data members.
    mutable hep::concurrency::RecursiveMutex mutex_{"GlobalSignal::mutex_"};

    // The registered signal handlers.
    std::deque<slot_type> signal_;
  };

  // 1.
  template <detail::SignalResponseType SRTYPE,
            typename ResultType,
            typename... Args>
  void
  GlobalSignal<SRTYPE, ResultType(Args...)>::watch(
    std::function<ResultType(Args...)> slot)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    detail::connect_to_signal<SRTYPE>(signal_, slot);
  }

  // 2a.
  template <detail::SignalResponseType SRTYPE,
            typename ResultType,
            typename... Args>
  template <typename T>
  void
  GlobalSignal<SRTYPE, ResultType(Args...)>::watch(
    ResultType (T::*slot)(Args...),
    T& t)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    watch(detail::makeWatchFunc(slot, t));
  }

  // 2b.
  template <detail::SignalResponseType SRTYPE,
            typename ResultType,
            typename... Args>
  template <typename T>
  void
  GlobalSignal<SRTYPE, ResultType(Args...)>::watch(
    T* t,
    ResultType (T::*slot)(Args...))
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    watch(detail::makeWatchFunc(slot, *t));
  }

  // 3a.
  template <detail::SignalResponseType SRTYPE,
            typename ResultType,
            typename... Args>
  template <typename T>
  void
  GlobalSignal<SRTYPE, ResultType(Args...)>::watch(
    ResultType (T::*slot)(Args...) const,
    T const& t)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    watch(detail::makeWatchFunc(slot, t));
  }

  // 3b.
  template <detail::SignalResponseType SRTYPE,
            typename ResultType,
            typename... Args>
  template <typename T>
  void
  GlobalSignal<SRTYPE, ResultType(Args...)>::watch(
    T const* t,
    ResultType (T::*slot)(Args...) const)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    watch(detail::makeWatchFunc(slot, *t));
  }

  template <detail::SignalResponseType SRTYPE,
            typename ResultType,
            typename... Args>
  void
  GlobalSignal<SRTYPE, ResultType(Args...)>::invoke(Args&&... args) const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto f : signal_) {
      f(std::forward<Args>(args)...);
    }
  }

} // namespace art
#endif /* art_Framework_Services_Registry_GlobalSignal_h */

// Local Variables:
// mode: c++
// End:
