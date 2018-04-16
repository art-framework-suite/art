#ifndef art_Framework_Services_Registry_LocalSignal_h
#define art_Framework_Services_Registry_LocalSignal_h

////////////////////////////////////////////////////////////////////////
// LocalSignal.h
//
// Define a wrapper for local signals. The watch(...) functions are for
// users wishing to register for callbacks; the invoke() and clear()
// functions are intended to be called only by art code.
//
// Multi-threaded behavior:
//
// watch...() access should be sequential only by construction since
// watch() calls are made from service constructors. Other calling
// origins are not supported.
//
// invoke...() access should be only from within the correct schedule's
// context so lock-free access to the correct signal is automatic and
// safe.
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Services/Registry/detail/SignalResponseType.h"
#include "art/Framework/Services/Registry/detail/makeWatchFunc.h"
#include "art/Utilities/ScheduleID.h"
#include "cetlib/container_algorithms.h"

#include <deque>
#include <functional>

namespace art {
  template <detail::SignalResponseType, typename ResultType, typename... Args>
  class LocalSignal;

  template <detail::SignalResponseType STYPE,
            typename ResultType,
            typename... Args>
  class LocalSignal<STYPE, ResultType(Args...)> {
  public:
    // Typedefs
    using slot_type = std::function<ResultType(Args...)>;
    using result_type = ResultType;

  private:
    // Required for derivative alias below.
    using ContainerType_ = std::vector<std::deque<slot_type>>;

  public:
    using size_type = typename ContainerType_::size_type;

    // Constructor.
    LocalSignal(size_t nSchedules);

    // 1. Free function or functor (or pre-bound member function).
    void watch(ScheduleID const, std::function<ResultType(Args...)> slot);
    // 2. Non-const member function.
    template <typename T>
    void watch(ScheduleID const, ResultType (T::*slot)(Args...), T& t);
    // 3. Const member function.
    template <typename T>
    void watch(ScheduleID const,
               ResultType (T::*slot)(Args...) const,
               T const& t);

    // 1. Free function or functor (or pre-bound member function).
    void watchAll(std::function<ResultType(Args...)> slot);
    // 2. Non-const member function.
    template <typename T>
    void watchAll(ResultType (T::*slot)(Args...), T& t);
    // 3. Const member function.
    template <typename T>
    void watchAll(ResultType (T::*slot)(Args...) const, T const& t);

    void invoke(ScheduleID const, Args&&... args) const; // Discard ResultType.

    void clear(ScheduleID const);
    void clearAll();

  private:
    ContainerType_ signals_;
  };

  template <detail::SignalResponseType STYPE,
            typename ResultType,
            typename... Args>
  LocalSignal<STYPE, ResultType(Args...)>::LocalSignal(size_t nSchedules)
    : signals_(nSchedules)
  {}

  // 1.
  template <detail::SignalResponseType STYPE,
            typename ResultType,
            typename... Args>
  void
  LocalSignal<STYPE, ResultType(Args...)>::watch(
    ScheduleID const sID,
    std::function<ResultType(Args...)> slot)
  {
    detail::connect_to_signal<STYPE>(signals_.at(sID.id()), slot);
  }

  // 2.
  template <detail::SignalResponseType STYPE,
            typename ResultType,
            typename... Args>
  template <typename T>
  void
  LocalSignal<STYPE, ResultType(Args...)>::watch(ScheduleID const sID,
                                                 ResultType (T::*slot)(Args...),
                                                 T& t)
  {
    watch(sID, detail::makeWatchFunc(slot, t));
  }

  // 3.
  template <detail::SignalResponseType STYPE,
            typename ResultType,
            typename... Args>
  template <typename T>
  void
  LocalSignal<STYPE, ResultType(Args...)>::watch(ScheduleID const sID,
                                                 ResultType (T::*slot)(Args...)
                                                   const,
                                                 T const& t)
  {
    watch(sID, detail::makeWatchFunc(slot, t));
  }

  // 1.
  template <detail::SignalResponseType STYPE,
            typename ResultType,
            typename... Args>
  void
  LocalSignal<STYPE, ResultType(Args...)>::watchAll(
    std::function<ResultType(Args...)> slot)
  {
    for (auto& signal : signals_) {
      detail::connect_to_signal<STYPE>(signal, slot);
    }
  }

  // 2.
  template <detail::SignalResponseType STYPE,
            typename ResultType,
            typename... Args>
  template <typename T>
  void
  LocalSignal<STYPE, ResultType(Args...)>::watchAll(
    ResultType (T::*slot)(Args...),
    T& t)
  {
    watchAll(detail::makeWatchFunc(slot, t));
  }

  // 3.
  template <detail::SignalResponseType STYPE,
            typename ResultType,
            typename... Args>
  template <typename T>
  void
  LocalSignal<STYPE, ResultType(Args...)>::watchAll(
    ResultType (T::*slot)(Args...) const,
    T const& t)
  {
    watchAll(detail::makeWatchFunc(slot, t));
  }

  template <detail::SignalResponseType STYPE,
            typename ResultType,
            typename... Args>
  void
  LocalSignal<STYPE, ResultType(Args...)>::invoke(ScheduleID const sID,
                                                  Args&&... args) const
  {
    for (auto f : signals_.at(sID.id())) {
      f(std::forward<Args>(args)...);
    }
  }

  template <detail::SignalResponseType STYPE,
            typename ResultType,
            typename... Args>
  void
  LocalSignal<STYPE, ResultType(Args...)>::clear(ScheduleID const sID)
  {
    signals_.at(sID.id()).clear();
  }

  template <detail::SignalResponseType STYPE,
            typename ResultType,
            typename... Args>
  void
  LocalSignal<STYPE, ResultType(Args...)>::clearAll()
  {
    for (auto& signal : signals_) {
      signal.clear();
    }
  }

} // namespace art
#endif /* art_Framework_Services_Registry_LocalSignal_h */

// Local Variables:
// mode: c++
// End:
