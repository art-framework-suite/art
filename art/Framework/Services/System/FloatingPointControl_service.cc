#pragma GCC diagnostic ignored "-Wunused-function"

#include "art/Framework/Services/System/FloatingPointControl.h"

// for the MMU (SSE), here are the bit definitions
// Pnemonic Bit Location Description
// FZ bit 15 Flush To Zero
#define ART_FZ 0x8000
// R+ bit 14 Round Positive
#define ART_R_PLUS 0x4000
// R- bit 13 Round Negative
#define ART_R_MINUS 0x2000
// RZ bits 13 and 14 Round To Zero
#define ART_RZ (ART_R_PLUS|ART_R_MINUS)
// RN bits 13 and 14 are 0 Round To Nearest
#define ART_RN_MASK ART_RZ
// PM bit 12 Precision Mask
#define ART_PM_MASK (0x1000)
// UM bit 11 Underflow Mask
#define ART_UM_MASK (0x0800)
// OM bit 10 Overflow Mask
#define ART_OM_MASK (0x0400)
// ZM bit 9 Divide By Zero Mask
#define ART_ZM_MASK (0x0200)
// DM bit 8 Denormal Mask
#define ART_DM_MASK (0x0100)
// IM bit 7 Invalid Operation Mask
#define ART_IM_MASK (0x0080)
// DAZ bit 6 Denormals Are Zero
#define ART_DAX 0x0040
// PE bit 5 Precision Flag
#define ART_PE 0x0020
// UE bit 4 Underflow Flag
#define ART_UE 0x0010
// OE bit 3 Overflow Flag
#define ART_OE 0x0008
// ZE bit 2 Divide By Zero Flag
#define ART_ZE 0x0004
// DE bit 1 Denormal Flag
#define ART_DE 0x0002
// IE bit 0 Invalid Operation Flag
#define ART_IE 0x0001


extern "C" {
#include <fenv.h>

#if (defined(__i386__) || defined(__x86_64__))
#ifdef __linux__
#include <fpu_control.h>
// #define SET_CONTROL_EX(env,ex) env.__control_word &= ~(ex)
#define SET_CONTROL_EX(env,ex) env.__control_word = (ex)
#else
  // works on MAC OS X
#include <architecture/i386/fpu.h>
#ifndef __GNUC__
#pragma STDC FENV_ACCESS ON
#endif
// #define SET_CONTROL_EX(env,ex) env.__control &= ~(ex)
#define SET_CONTROL_EX(env,ex) env.__control = (ex)
#endif
#else
#error architecture not valid
#endif
}

static  char const *
on_or_off [[gnu::unused]] ( bool b )
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
  reg.sPostEndJob.watch(this, & FloatingPointControl::postEndJob);
  reg.sPreModule.watch (this, & FloatingPointControl::preModule);
  reg.sPostModule.watch(this, & FloatingPointControl::postModule);

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
  FloatingPointControl::postModule(const ModuleDescription&)
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
  unsigned short int FE_PRECISION __attribute__((unused)) = 1<<5;
/*
 * NB: We do not let users control signaling inexact (FE_INEXACT).
 */

  unsigned short int suppress = FE_PRECISION;
  unsigned short int enable_sse = 0;

  if ( !enableDivByZeroEx_ )
    suppress |= FE_DIVBYZERO;
  else
    enable_sse |= ART_ZM_MASK;

  if ( !enableInvalidEx_ )
    suppress |= FE_INVALID;
  else
    enable_sse |= ART_IM_MASK;

  if ( !enableOverFlowEx_ )
    suppress |= FE_OVERFLOW;
  else
    enable_sse |= ART_OM_MASK;

  if ( !enableUnderFlowEx_ )
    suppress |= FE_UNDERFLOW;
  else
    enable_sse |= ART_UM_MASK;

  fegetenv( &fpuState_ );
  SET_CONTROL_EX(fpuState_, suppress);
  // also set the bits in the SSE unit
  fpuState_.__mxcsr &= ~enable_sse;
  // fpuState_.__mxcsr = ~supp_sse;

  // old way of directly setting it
  // fpuState_.__control_word = suppress;

  fesetenv( &fpuState_ );

#ifdef __linux__
#ifdef __i386__
  if (setPrecisionDouble_) {
    fpu_control_t cw;
    _FPU_GETCW(cw);

    cw = (cw & ~_FPU_EXTENDED) | _FPU_DOUBLE;
    _FPU_SETCW(cw);
  }
#endif
#endif

}

// ----------------------------------------------------------------------

void
  FloatingPointControl::echoState( )
{
#ifdef __linux__
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
#endif
}

// ======================================================================

PROVIDE_FILE_PATH()

// ======================================================================
