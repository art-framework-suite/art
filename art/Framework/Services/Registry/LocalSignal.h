#ifndef art_Framework_Services_Registry_LocalSignal_h
#define art_Framework_Services_Registry_LocalSignal_h

// Define a wrapper for local signals. The invoke() and clear() signals
// are intended to be called only be art::ActivityRegistry.

// Multi-threaded behavior:
//
// watch...() access should be sequential only by construction since
// watch() calls are made from service constructors. Other calling
// origins are not supported.
//
// invoke...() access should be only from within the correct schedule's
// context so lock-free access to the correct signal is automatic and
// safe.
#include "art/Framework/Services/Registry/detail/SignalResponseType.h"
#include "art/Utilities/ScheduleID.h"
#include "cetlib/container_algorithms.h"
#include "cpp0x/functional"
#include "sigc++/signal.h"

#if ! ( defined (ART_NO_FIXED_VARIADIC_EXPANSION) || (GCC_IS_AT_LEAST(4, 7, 0)) )
#define ART_NO_FIXED_VARIADIC_EXPANSION
#endif

namespace art {
#ifdef ART_NO_FIXED_VARIADIC_EXPANSION
  template <detail::SignalResponseType STYPE, typename ReturnType, typename A1 = void, typename A2 = void> class LocalSignal;
  template <detail::SignalResponseType STYPE, typename ReturnType, typename A1> class LocalSignal<STYPE, ReturnType, A1>;
  template <detail::SignalResponseType STYPE, typename ReturnType> class LocalSignal<STYPE, ReturnType>;
#else
  template <detail::SignalResponseType, typename ReturnType, typename... Args > class LocalSignal;
#endif
}

#ifdef ART_NO_FIXED_VARIADIC_EXPANSION
// Zero argument callbacks.
template <art::detail::SignalResponseType STYPE, typename ReturnType>
class art::LocalSignal<STYPE, ReturnType> {
private:
  typedef sigc::signal<ReturnType> SigType_;
  typedef std::vector<SigType_> ContainerType_;
public:
  typedef typename ContainerType_::size_type size_type;
  // Constructor.
  LocalSignal(size_t nSchedules)
    :
    signals_(nSchedules)
    {
    }

  // 1. Free function or functor (or pre-bound member function).
  void
  watch(ScheduleID sID, std::function < ReturnType() > slot) {
    detail::connect_to_signal<STYPE>(signals_.at(sID.id()), slot);
  }
  // 2. Non-const member function.
  template <typename T>
  void
  watch(ScheduleID sID, ReturnType(T::*slot)(), T & t) {
    watch(std::bind(slot, t));
  }
  // 3. Const member function.
  template <typename T>
  void
  watch(ScheduleID sID, ReturnType(T::*slot)() const, T const & t) {
    watch(std::bind(slot, t));
  }

  // 1. Free function or functor (or pre-bound member function).
  void
  watchAll(std::function < ReturnType() > slot) {
    for ( auto & signal : signals_ ) {
      detail::connect_to_signal<STYPE>(signal, slot);
    }
  }
  // 2. Non-const member function.
  template <typename T>
  void
  watchAll(ReturnType(T::*slot)(), T & t) {
    watchAll(std::bind(slot, t));
  }
  // 3. Const member function.
  template <typename T>
  void
  watchAll(ReturnType(T::*slot)() const, T const & t) {
    watchAll(std::bind(slot, t));
  }

  ReturnType invoke(ScheduleID sID) const {
    return signals_.at(sID.id())();
  }
  void clear(ScheduleID sID) {
    signals_.at(sID.id()).clear();
  }
  void clearAll() {
    for ( auto & signal : signals_ ) {
      signal.clear();
    }
  }

private:
  ContainerType_ signals_;
};
// One argument callbacks.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename A1>
class art::LocalSignal<STYPE, ReturnType, A1> {
private:
  typedef sigc::signal<ReturnType, A1> SigType_;
  typedef std::vector<SigType_> ContainerType_;
public:
  typedef typename ContainerType_::size_type size_type;
  // Constructor.
  LocalSignal(size_t nSchedules)
    :
    signals_(nSchedules)
    {
    }

  // 1. Free function or functor (or pre-bound member function).
  void
  watch(ScheduleID sID, std::function < ReturnType(A1) > slot) {
    detail::connect_to_signal<STYPE>(signals_.at(sID.id()), slot);
  }
  // 2. Non-const member function.
  template <typename T>
  void
  watch(ScheduleID sID, ReturnType(T::*slot)(A1), T & t)
  {
    watch(std::bind(slot, t));
  }
  // 3. Const member function.
  template <typename T>
  void
  watch(ScheduleID sID, ReturnType(T::*slot)(A1) const, T const & t)
  {
    watch(std::bind(slot, t));
  }

  // 1. Free function or functor (or pre-bound member function).
  void
  watchAll(std::function < ReturnType(A1) > slot)
  {
    for ( auto & signal : signals_ ) {
      detail::connect_to_signal<STYPE>(signal, slot);
    }
  }
  // 2. Non-const member function.
  template <typename T>
  void
  watchAll(ReturnType(T::*slot)(A1), T & t)
  {
    watchAll(std::bind(slot, t));
  }
  // 3. Const member function.
  template <typename T>
  void
  watchAll(ReturnType(T::*slot)(A1) const, T const & t)
  {
    watchAll(std::bind(slot, t));
  }

  ReturnType invoke(ScheduleID sID, A1 a1) const
  {
    return signals_.at(sID.id())(a1);
  }
  void clear(ScheduleID sID)
  {
    signals_.at(sID.id()).clear();
  }
  void clearAll()
  {
    for ( auto & signal : signals_ ) {
      signal.clear();
    }
  }

private:
  ContainerType_ signals_;
};
// Two argument callbacks.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename A1, typename A2>
class art::LocalSignal {
private:
  typedef sigc::signal<ReturnType, A1, A2> SigType_;
  typedef std::vector<SigType_> ContainerType_;
public:
  typedef typename ContainerType_::size_type size_type;
  // Constructor.
  LocalSignal(size_t nSchedules)
    :
    signals_(nSchedules)
    {
    }

  // 1. Free function or functor (or pre-bound member function).
  void
  watch(ScheduleID sID, std::function < ReturnType(A1, A2) > slot)
  {
    detail::connect_to_signal<STYPE>(signals_.at(sID.id()), slot);
  }
  // 2. Non-const member function.
  template <typename T>
  void
  watch(ScheduleID sID, ReturnType(T::*slot)(A1, A2), T & t)
  {
    watch(std::bind(slot, t));
  }
  // 3. Const member function.
  template <typename T>
  void
  watch(ScheduleID sID, ReturnType(T::*slot)(A1, A2) const, T const & t)
  {
    watch(std::bind(slot, t));
  }

  // 1. Free function or functor (or pre-bound member function).
  void
  watchAll(std::function < ReturnType(A1, A2) > slot)
  {
    for ( auto & signal : signals_ ) {
      detail::connect_to_signal<STYPE>(signal, slot);
    }
  }
  // 2. Non-const member function.
  template <typename T>
  void
  watchAll(ReturnType(T::*slot)(A1, A2), T & t)
  {
    watchAll(std::bind(slot, t));
  }
  // 3. Const member function.
  template <typename T>
  void
  watchAll(ReturnType(T::*slot)(A1, A2) const, T const & t)
  {
    watchAll(std::bind(slot, t));
  }

  ReturnType invoke(ScheduleID sID, A1 a1, A2 a2) const
  {
    return signals_.at(sID.id())(a1, a2);
  }
  void clear(ScheduleID sID)
  {
    signals_.at(sID.id()).clear();
  }
  void clearAll()
  {
    for ( auto & signal : signals_ ) {
      signal.clear();
    }
  }

private:
  ContainerType_ signals_;
};
#else
// For when variadic templates work per C++ 2011.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename...Args>
class art::LocalSignal {
private:
  typedef sigc::signal<ReturnType, Args...> SigType_;
  typedef std::vector<SigType_> ContainerType_;
public:
  typedef typename ContainerType_::size_type size_type;
  // Constructor.
  LocalSignal(size_t nSchedules);

  // 1. Free function or functor (or pre-bound member function).
  void
  watch(ScheduleID sID, std::function < ReturnType(Args...) > slot);
  // 2. Non-const member function.
  template <typename T>
  void
  watch(ScheduleID sID, ReturnType(T::*slot)(Args...), T & t);
  // 3. Const member function.
  template <typename T>
  void
  watch(ScheduleID sID, ReturnType(T::*slot)(Args...) const, T const & t);

  // 1. Free function or functor (or pre-bound member function).
  void
  watchAll(std::function < ReturnType(Args...) > slot);
  // 2. Non-const member function.
  template <typename T>
  void
  watchAll(ReturnType(T::*slot)(Args...), T & t);
  // 3. Const member function.
  template <typename T>
  void
  watchAll(ReturnType(T::*slot)(Args...) const, T const & t);

  ReturnType invoke(ScheduleID sID, Args... args) const;
  void clear(ScheduleID sID);
  void clearAll();

private:
  ContainerType_ signals_;
};

template <art::detail::SignalResponseType STYPE, typename ReturnType, typename...Args>
art::LocalSignal<STYPE, ReturnType, Args...>::
LocalSignal(size_t nSchedules)
:
  signals_(nSchedules)
{
}

// 1.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename...Args>
void
art::LocalSignal<STYPE, ReturnType, Args...>::
watch(ScheduleID sID, std::function < ReturnType(Args...) > slot)
{
  detail::connect_to_signal<STYPE>(signals_.at(sID.id()),  slot);
}

// 2.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename...Args>
void
art::LocalSignal<STYPE, ReturnType, Args...>::
watch(ScheduleID sID, ReturnType(T::*slot)(Args...), T & t)
{
  watch(std::bind(slot, t));

}

// 3.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename...Args>
void
art::LocalSignal<STYPE, ReturnType, Args...>::
watch(ScheduleID sID, ReturnType(T::*slot)(Args...) const, T const & t)
{
  watch(std::bind(slot, t));
}

// 1.
template <typename ReturnType, typename... Args>
void
art::LocalSignal<ReturnType, Args...>::
watchAll(std::function<ReturnType(Args...)> slot)
{
  for ( auto & signal : signals_ ) {
    detail::connect_to_signal<STYPE>(signal, slot);
  }
}

// 2.
template <typename ReturnType, typename... Args>
template <typename T>
void
art::LocalSignal<ReturnType, Args...>::
watchAll(ReturnType(T::*slot)(Args...), T & t)
{
  watchAll(std::bind(slot, t));
}

// 3.
template <typename ReturnType, typename... Args>
template <typename T>
void
art::LocalSignal<ReturnType, Args...>::
watchAll(ReturnType(T::*slot)(Args...) const, T const & t)
{
  watchAll(std::bind(slot, t));
}

template <typename ReturnType, typename... Args>
ReturnType
art::LocalSignal<ReturnType, Args...>::
invoke(ScheduleID sID, Args... args)
{
  return signals_.at(sID.id())(std::forward<Args>(args)...);
}

template <typename ReturnType, typename... Args>
void
art::LocalSignal<ReturnType, Args...>::
clear(ScheduleID sID)
{
  signals_.at(sID.id()).clear();
}

template <typename ReturnType, typename... Args>
void
art::LocalSignal<ReturnType, Args...>::
clearAll()
{
  for ( auto & signal : signals_ ) {
    signal.clear();
  }
}
#endif

#endif /* art_Framework_Services_Registry_LocalSignal_h */

// Local Variables:
// mode: c++
// End:
