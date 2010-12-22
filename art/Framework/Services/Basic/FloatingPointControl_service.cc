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
#include "art/Framework/Services/Registry/ServiceMaker.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <fenv.h>
#include <map>
#include <stack>

#ifdef __linux__
#ifdef __i386__
#include <fpu_control.h>
#endif  // __i386__
#endif  // __linux__

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

static  char const *
  on_or_off( bool b )
{
  static char const * on  = " on";
  static char const * off = " off";

  return b ? on : off;
}

// ======================================================================

FloatingPointControl::FloatingPointControl( ParameterSet const & cfg
                                          , ActivityRegistry   & reg
                                          )
: enableDivByZeroEx_ ( false )
, enableInvalidEx_   ( false )
, enableOverFlowEx_  ( false )
, enableUnderFlowEx_ ( false )
, setPrecisionDouble_( cfg.get<bool>("setPrecisionDouble",true) )
, reportSettings_    ( cfg.get<bool>("reportSettings",false) )
, fpuState_          ( )
, OSdefault_         ( )
, stateMap_          ( )
, stateStack_        ( )
{
  reg.watchPostEndJob(this, & FloatingPointControl::postEndJob);
  reg.watchPreModule (this, & FloatingPointControl::preModule);
  reg.watchPostModule(this, & FloatingPointControl::postModule);

  // Get the state of the fpu and save it as the "OSdefault" state.
  // The language here is a bit odd.  We use "OSdefault" to label the fpu
  // state we inherit from the OS on job startup.  By contrast, "default"
  // is the label we use for the fpu state when either we or the user has
  // specified a state for modules not appearing in the module list.
  // Generally, "OSdefault" and "default" are the same but are not
  // required to be so.

  fegetenv( &fpuState_ );
  OSdefault_ = fpuState_;
  stateStack_.push(OSdefault_);
  if( reportSettings_ )  {
    mf::LogVerbatim("FPE_Enable") << "\nSettings for OSdefault";
    echoState();
  }

  // Then go handle the cases described in the cfg file

  PSet    empty_PSet;
  VString empty_VString;

  VString moduleNames = cfg.get<std::vector<std::string> >("moduleNames",empty_VString);

  // If the module name list is missing or empty,
  // set default values for all parameters

  if( moduleNames.empty() ) {
    enableDivByZeroEx_  = false;
    enableInvalidEx_    = false;
    enableOverFlowEx_   = false;
    enableUnderFlowEx_  = false;
    setPrecisionDouble_ = true;

    controlFpe();
    if( reportSettings_ ) {
      mf::LogVerbatim("FPE_Enable") << "\nSettings for default";
      echoState();
    }
    fegetenv( &fpuState_ );
    stateMap_["default"] =  fpuState_;

  } else {

    // Otherwise, scan the module name list and set per-module values.
    // Be careful to treat any user-specified default first.  If there
    // is one, use it to override our default.  Then remove it from the
    // list so we don't see it again while handling everything else.

    //VString::iterator pos = find_in_all(moduleNames, "default");
    VString::iterator pos = find( moduleNames.begin(), moduleNames.end()
                                , "default"
                                );
    if( pos != moduleNames.end() ) {
      PSet secondary = cfg.get<fhicl::ParameterSet>(*pos, empty_PSet);
      enableDivByZeroEx_ = secondary.get<bool>("enableDivByZeroEx", false);
      enableInvalidEx_   = secondary.get<bool>("enableInvalidEx"  , false);
      enableOverFlowEx_  = secondary.get<bool>("enableOverFlowEx" , false);
      enableUnderFlowEx_ = secondary.get<bool>("enableUnderFlowEx", false);
      controlFpe();
      if( reportSettings_ ) {
        mf::LogVerbatim("FPE_Enable") << "\nSettings for unnamed module";
        echoState();
      }
      fegetenv( &fpuState_ );
      stateMap_["default"] =  fpuState_;
      moduleNames.erase(pos);
    }

  // Then handle the rest.

    for( VString::const_iterator it(moduleNames.begin())
                               , itEnd = moduleNames.end(); it != itEnd; ++it) {
      PSet secondary = cfg.get<fhicl::ParameterSet>(*it, empty_PSet);
      enableDivByZeroEx_  = secondary.get<bool>("enableDivByZeroEx", false);
      enableInvalidEx_    = secondary.get<bool>("enableInvalidEx",   false);
      enableOverFlowEx_   = secondary.get<bool>("enableOverFlowEx",  false);
      enableUnderFlowEx_  = secondary.get<bool>("enableUnderFlowEx", false);
      controlFpe();
      if( reportSettings_ ) {
        mf::LogVerbatim("FPE_Enable") << "\nSettings for module " << *it;
        echoState();
      }
      fegetenv( &fpuState_ );
      stateMap_[*it] =  fpuState_;
    }
  }

  // And finally, restore the state back to the way we found it originally

    fesetenv( &OSdefault_ );
}  // c'tor

// ----------------------------------------------------------------------

void
  FloatingPointControl::postEndJob( )
{
  // At EndJob, put the state of the fpu back to "OSdefault"
  fpuState_ = stateMap_[String("OSdefault")];
  fesetenv( &OSdefault_ );
  if( reportSettings_ ) {
    mf::LogVerbatim("FPE_Enable") << "\nSettings at end job ";
    echoState();
  }
}

// ----------------------------------------------------------------------

void
  FloatingPointControl::preModule(const ModuleDescription& iDescription)
{
  // On entry to a module, find the desired state of the fpu and set it
  // accordingly.  Note that any module whose label does not appear in our
  // list gets the default settings.

  String modName = iDescription.moduleLabel();
  if( stateMap_.find(modName) == stateMap_.end() )  {
    fpuState_ = stateMap_[String("default")];
  } else {
    fpuState_ = stateMap_[modName];
  }
  fesetenv( &fpuState_ );
  stateStack_.push(fpuState_);
  if( reportSettings_ ) {
    mf::LogVerbatim("FPE_Enable") << "\nSettings at begin module " << modName;
    echoState();
  }
}

// ----------------------------------------------------------------------

void
  FloatingPointControl::postModule(const ModuleDescription& iDescription)
{

// On exit from a module, set the state of the fpu back to what it was before entry

  stateStack_.pop();
  fpuState_ = stateStack_.top();
  fesetenv( &fpuState_ );
  if( reportSettings_ ) {
    mf::LogVerbatim("FPE_Enable") << "\nSettings after end module ";
    echoState();
  }
}

// ----------------------------------------------------------------------

void
  FloatingPointControl::controlFpe( )
{
  unsigned short int FE_PRECISION = 1<<5;
  unsigned short int suppress;

#ifdef __linux__

/*
 * NB: We do not let users control signaling inexact (FE_INEXACT).
 */

  suppress = FE_PRECISION;
  if ( !enableDivByZeroEx_ ) suppress |= FE_DIVBYZERO;
  if ( !enableInvalidEx_ )   suppress |= FE_INVALID;
  if ( !enableOverFlowEx_ )  suppress |= FE_OVERFLOW;
  if ( !enableUnderFlowEx_ ) suppress |= FE_UNDERFLOW;
  fegetenv( &fpuState_ );
  fpuState_.__control_word = suppress;
  fesetenv( &fpuState_ );

#ifdef __i386__
  if (setPrecisionDouble_) {
    fpu_control_t cw;
    _FPU_GETCW(cw);

    cw = (cw & ~_FPU_EXTENDED) | _FPU_DOUBLE;
    _FPU_SETCW(cw);
  }
#endif  // __i386__
#endif  // __linux__

}

// ----------------------------------------------------------------------

void
  FloatingPointControl::echoState( )
{
  if( reportSettings_ ) {
    int femask = fegetexcept();
    mf::LogVerbatim("FPE_Enable")
      << "Floating point exception mask is "
      << std::showbase << std::hex << femask
      << "\tDivByZero exception is" << on_or_off(femask & FE_DIVBYZERO)
      << "\tInvalid exception is"   << on_or_off(femask & FE_INVALID  )
      << "\tOverFlow exception is"  << on_or_off(femask & FE_OVERFLOW )
      << "\tUnderFlow exception is" << on_or_off(femask & FE_UNDERFLOW)
      ;
  }
}

// ======================================================================

DEFINE_ART_SERVICE(FloatingPointControl);
