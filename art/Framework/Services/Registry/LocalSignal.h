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

namespace art {
  template <detail::SignalResponseType, typename ReturnType, typename... Args > class LocalSignal;
}

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
template <typename T>
void
art::LocalSignal<STYPE, ReturnType, Args...>::
watch(ScheduleID sID, ReturnType(T::*slot)(Args...), T & t)
{
  watch(std::bind(slot, t));

}

// 3.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename...Args>
template <typename T>
void
art::LocalSignal<STYPE, ReturnType, Args...>::
watch(ScheduleID sID, ReturnType(T::*slot)(Args...) const, T const & t)
{
  watch(std::bind(slot, t));
}

// 1.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename... Args>
void
art::LocalSignal<STYPE, ReturnType, Args...>::
watchAll(std::function<ReturnType(Args...)> slot)
{
  for ( auto & signal : signals_ ) {
    detail::connect_to_signal<STYPE>(signal, slot);
  }
}

// 2.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename... Args>
template <typename T>
void
art::LocalSignal<STYPE, ReturnType, Args...>::
watchAll(ReturnType(T::*slot)(Args...), T & t)
{
  watchAll(std::bind(slot, t));
}

// 3.
template <art::detail::SignalResponseType STYPE, typename ReturnType, typename... Args>
template <typename T>
void
art::LocalSignal<STYPE, ReturnType, Args...>::
watchAll(ReturnType(T::*slot)(Args...) const, T const & t)
{
  watchAll(std::bind(slot, t));
}

template <art::detail::SignalResponseType STYPE, typename ReturnType, typename... Args>
ReturnType
art::LocalSignal<STYPE, ReturnType, Args...>::
invoke(ScheduleID sID, Args... args) const
{
  return signals_.at(sID.id())(std::forward<Args>(args)...);
}

template <art::detail::SignalResponseType STYPE, typename ReturnType, typename... Args>
void
art::LocalSignal<STYPE, ReturnType, Args...>::
clear(ScheduleID sID)
{
  signals_.at(sID.id()).clear();
}

template <art::detail::SignalResponseType STYPE, typename ReturnType, typename... Args>
void
art::LocalSignal<STYPE, ReturnType, Args...>::
clearAll()
{
  for ( auto & signal : signals_ ) {
    signal.clear();
  }
}
#endif /* art_Framework_Services_Registry_LocalSignal_h */

// Local Variables:
// mode: c++
// End:
