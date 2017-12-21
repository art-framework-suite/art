#ifndef art_Framework_Core_ProducingServiceSignals_h
#define art_Framework_Core_ProducingServiceSignals_h

////////////////////////////////////////////////////////////////////////
// ProducingServiceSignals
//
// Registry holding the signals to which services may subscribe.
//
// Services can connect to the signals distributed by the
// ProducingServiceSignals in order to monitor the activity of the
// application.
//
// Signals are either global. Register a watchpoint by calling the
// watch() function of the appropriate signal.
//
//  GlobalSignal<detail::SignalResponseType::FIFO,
//  void(EventPrincipal&)> sPreModuleBeginJob;
//
// describes a watchpoint whose callable objects should have void return
// type and take a single argument of type EventPrincipal&.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/GlobalSignal.h"
#include "art/Framework/Services/Registry/detail/SignalResponseType.h"

namespace art {

  class EventPrincipal;
  class SubRunPrincipal;
  class RunPrincipal;

  class ProducingServiceSignals {
  public:
    explicit ProducingServiceSignals() = default;
    ProducingServiceSignals(ProducingServiceSignals const&) = delete;
    ProducingServiceSignals& operator=(ProducingServiceSignals const&) = delete;

    GlobalSignal<detail::SignalResponseType::LIFO, void(EventPrincipal&)>
      sPostReadEvent;
    GlobalSignal<detail::SignalResponseType::LIFO, void(SubRunPrincipal&)>
      sPostReadSubRun;
    GlobalSignal<detail::SignalResponseType::LIFO, void(RunPrincipal&)>
      sPostReadRun;
  };
}

#endif /* art_Framework_Core_ProducingServiceSignals_h */

// Local Variables:
// mode: c++
// End:
