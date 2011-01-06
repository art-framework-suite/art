#ifndef ART_FRAMEWORK_SERVICES_BASIC_FLOATINGPOINTCONTROL_H
#define ART_FRAMEWORK_SERVICES_BASIC_FLOATINGPOINTCONTROL_H
// ======================================================================
//
// FloatingPointControl - give users the ability to specify, on a
// per-module basis, the behavior of the Floating Point (FP) Processor.
//
// ======================================================================
//
// This service can control two separate aspects of the FP environment:
//   1. exceptions
//   2. precision control on x87 FP processors.
//
// If the service is not used, floating point exceptions will not be
// trapped anywhere (FP exceptions will not cause a crash).  To enable the
// exceptions, add something like the following to the configuration file:
//
// service = FloatingPointControl
//     {
//     untracked bool reportSettings = false
//
//     untracked vstring moduleNames = {  "default"
//                                       ,"sendMessages1"
//                                       ,"sendMessages2"  }
//
//     untracked PSet default = {
//                       untracked bool enableDivByZeroEx = false
//                       untracked bool enableInvalidEx = false
//                       untracked bool enableOverFlowEx = false
//                       untracked bool enableUnderFlowEx = false
//                       }
//
//     untracked PSet sendMessages1 = {
//                       untracked bool enableDivByZeroEx = true
//                       untracked bool enableInvalidEx = false
//                       untracked bool enableOverFlowEx = false
//                       untracked bool enableUnderFlowEx = false
//                       }
//
//    untracked PSet sendMessages2 = {
//                       untracked bool enableDivByZeroEx = false
//                       untracked bool enableInvalidEx = true
//                       untracked bool enableOverFlowEx = false
//                       untracked bool enableUnderFlowEx = false
//                       }
//   }
//
//   path p = { sendMessages1, sendMessages2, sendMessages3  }
//
//   module sendMessages1 = makeSignals1 { }
//   module sendMessages2 = makeSignals2 { }
//   module sendMessages3 = makeSignals3 { }
//
// In this example, the DivideByZero exception is enabled only for the
// module with label sendMessages1, the Invalid exception is enabled only
// for the module with label sendMessages2 and no floating point exceptions
// are otherwise enabled.  The defaults for these options are currently all
// false.  (in an earlier version DivByZero, Invalid, and Overflow defaulted
// to true; we hope to return to those defaults someday when the frequency
// of such exceptions has decreased)
//
// Enabling exceptions is very useful if you are trying to track down where a
// floating point value of 'nan' or 'inf' is being generated and is even
// better if the goal is to eliminate them.
//
// One can also control the precision of floating point operations in x87 FP
// processor.
//
// service = FloatingPointControl
//  {
//     untracked bool setPrecisionDouble = true
//  }
//
// If set true (the default if the service is used), the floating precision
// in theq x87 math processor will be set to round results of addition,
// subtraction, multiplication, division, and square root to 64 bits after
// each operation instead of the x87 default, which is 80 bits for values
// in registers (this is the default you get if this service is not used at
// all).
//
// The precision control only affects Intel and AMD 32 bit CPUs under LINUX.
// We have not implemented precision control in the service for other CPUs
// (some other CPUs round to 64 bits by default and often CPUs do not allow
// control of the precision of floating point calculations).
//
// ======================================================================

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <fenv.h>
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

using art::FloatingPointControl;
using fhicl::ParameterSet;

// ----------------------------------------------------------------------

class art::FloatingPointControl
{
  // non-copyable:
  FloatingPointControl( FloatingPointControl const & );
  void  operator = ( FloatingPointControl const & );

public:
  // c'tor:
  FloatingPointControl( ParameterSet const &
                               , ActivityRegistry   &
                               );

  // use compiler-generated d'tor

  void postEndJob( );

  void preModule(const ModuleDescription&);
  void postModule(const ModuleDescription&);

private:
  void controlFpe( );
  void echoState( );

  bool enableDivByZeroEx_;
  bool enableInvalidEx_;
  bool enableOverFlowEx_;
  bool enableUnderFlowEx_;

  bool setPrecisionDouble_;
  bool reportSettings_;

  typedef  std::string          String;
  typedef  std::vector<String>  VString;
  typedef  fhicl::ParameterSet  PSet;

  fenv_t                    fpuState_;
  fenv_t                    OSdefault_; // OS's fpu state on job startup
  std::map<String, fenv_t>  stateMap_;
  std::stack<fenv_t>        stateStack_;

};  // FloatingPointControl

// ======================================================================

#endif
