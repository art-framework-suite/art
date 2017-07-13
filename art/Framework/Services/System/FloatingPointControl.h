#ifndef art_Framework_Services_System_FloatingPointControl_h
#define art_Framework_Services_System_FloatingPointControl_h

// ======================================================================
//
// FloatingPointControl
//
// This service gives users the ability to specify the behavior of the
// Floating Point (FP) Processor for the job.
//
// This service can control two separate aspects of the FP environment:
//   1. exceptions
//   2. precision control on x87 FP processors.
//
// If the service is not used, floating point exceptions will not be
// trapped anywhere (FP exceptions will not cause a crash).
//
// Enabling exceptions is very useful if you are trying to track down
// where a floating point value of 'nan' or 'inf' is being generated
// and is even better if the goal is to eliminate them.
//
// One can also control the precision of floating point operations in
// x87 FP processor by specifying:
//
//   setPrecisionDouble: (true|false)
//
// If set true (the default), the floating precision in the x87 math
// processor will be set to round results of addition, subtraction,
// multiplication, division, and square root to 64 bits after each
// operation instead of the x87 default, which is 80 bits for values
// in registers (this is the default you get if this service is not
// used at all).
//
// The precision control only affects Intel and AMD 32 bit CPUs under
// LINUX.  We have not implemented precision control in the service
// for other CPUs (some other CPUs round to 64 bits by default and
// often CPUs do not allow control of the precision of floating point
// calculations).
//
// ======================================================================

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "fhiclcpp/types/Atom.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

extern "C" {
#include "xpfpa/xpfpa.h"
#include <fenv.h>
}

#include <map>
#include <stack>

#ifdef __linux__
#ifdef __i386__
#include <fpu_control.h>
#endif
#endif

namespace art {
  class FloatingPointControl;
}

// ----------------------------------------------------------------------

class art::FloatingPointControl {
  FloatingPointControl(FloatingPointControl const&) = delete;
  FloatingPointControl& operator=(FloatingPointControl const&) = delete;
public:

  struct Config {
    fhicl::Atom<bool> enableDivByZeroEx {fhicl::Name{"enableDivByZeroEx"}, false};
    fhicl::Atom<bool> enableInvalidEx {fhicl::Name{"enableInvalidEx"}, false};
    fhicl::Atom<bool> enableOverFlowEx {fhicl::Name{"enableOverFlowEx"}, false};
    fhicl::Atom<bool> enableUnderFlowEx {fhicl::Name{"enableUnderFlowEx"}, false};
    fhicl::Atom<bool> setPrecisionDouble {fhicl::Name{"setPrecisionDouble"}, true};
    fhicl::Atom<bool> reportSettings {fhicl::Name{"reportSettings"}, false};
  };

  using Parameters = ServiceTable<Config>;
  explicit FloatingPointControl(Parameters const&, ActivityRegistry&);

private:

  void postEndJob();
  void controlFpe();
  void echoState();

  bool enableDivByZeroEx_;
  bool enableInvalidEx_;
  bool enableOverFlowEx_;
  bool enableUnderFlowEx_;
  bool setPrecisionDouble_;
  bool reportSettings_;

  fenv_t OSdefault_ {}; // OS's fpu state on job startup

  bool precisionSet_ {false}; // Keep track of whether we should restore

  XPFPA_DECLARE()
};  // FloatingPointControl

// ======================================================================

DECLARE_ART_SYSTEM_SERVICE(art::FloatingPointControl, LEGACY)
#endif /* art_Framework_Services_System_FloatingPointControl_h */

// Local Variables:
// mode: c++
// End:
