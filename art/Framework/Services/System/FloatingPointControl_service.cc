#include "art/Framework/Services/System/FloatingPointControl.h"
#include "cetlib/container_algorithms.h"

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
#define SET_CONTROL_EX(env,ex) env.__control_word = (ex)
#else
  // works on MAC OS X
#include <architecture/i386/fpu.h>
#ifndef __GNUC__
#pragma STDC FENV_ACCESS ON
#endif
#define SET_CONTROL_EX(env,ex) env.__control = (ex)
#endif
#else
#error architecture not valid
#endif
}

namespace {
  char const* on_or_off [[gnu::unused]] (bool const b)
  {
    return b ? " on" : " off";
  }
}

// ======================================================================

art::FloatingPointControl::FloatingPointControl(Parameters const& c, ActivityRegistry& reg)
  : enableDivByZeroEx_ {c().enableDivByZeroEx()}
  , enableInvalidEx_   {c().enableInvalidEx()}
  , enableOverFlowEx_  {c().enableOverFlowEx()}
  , enableUnderFlowEx_ {c().enableUnderFlowEx()}
  , setPrecisionDouble_{c().setPrecisionDouble()}
  , reportSettings_    {c().reportSettings()}
{
  reg.sPostEndJob.watch(this, &FloatingPointControl::postEndJob);

  // Get the state of the fpu to restore it after the job is done.
  fenv_t fpuState;
  fegetenv(&fpuState);
  OSdefault_ = fpuState;

  if (reportSettings_) {
    mf::LogVerbatim("FPE_Enable") << "\nOS's FP settings";
    echoState();
  }

  // Update the state according to the user's configuration
  controlFpe(fpuState);
  if (reportSettings_) {
    mf::LogVerbatim("FPE_Enable") << "\nUpdated FP settings per user's configuration";
    echoState();
  }
}

// ----------------------------------------------------------------------

void
art::FloatingPointControl::postEndJob()
{
  fesetenv(&OSdefault_);
  if (reportSettings_) {
    mf::LogVerbatim("FPE_Enable") << "\nRestored to OS's FPE settings";
    echoState();
  }
}

// ----------------------------------------------------------------------

void
art::FloatingPointControl::controlFpe(fenv_t& fpuState)
{
  unsigned short int const FE_PRECISION {1<<5};

  // NB: We do not let users control signaling inexact (FE_INEXACT).

  unsigned short int suppress {FE_PRECISION};
  unsigned short int enable_sse {};

  if (!enableDivByZeroEx_)
    suppress |= FE_DIVBYZERO;
  else
    enable_sse |= ART_ZM_MASK;

  if (!enableInvalidEx_)
    suppress |= FE_INVALID;
  else
    enable_sse |= ART_IM_MASK;

  if (!enableOverFlowEx_)
    suppress |= FE_OVERFLOW;
  else
    enable_sse |= ART_OM_MASK;

  if (!enableUnderFlowEx_)
    suppress |= FE_UNDERFLOW;
  else
    enable_sse |= ART_UM_MASK;

  fegetenv(&fpuState);
  SET_CONTROL_EX(fpuState, suppress);
  // also set the bits in the SSE unit
  fpuState.__mxcsr &= ~enable_sse;

  fesetenv(&fpuState);

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
art::FloatingPointControl::echoState()
{
#ifdef __linux__
  if (reportSettings_) {
    int const femask {fegetexcept()};
    mf::LogVerbatim("FPE_Enable")
      << "Floating point exception mask is "
      << std::showbase << std::hex << femask
      << "\tDivByZero exception is" << on_or_off(femask & FE_DIVBYZERO)
      << "\tInvalid exception is"   << on_or_off(femask & FE_INVALID)
      << "\tOverFlow exception is"  << on_or_off(femask & FE_OVERFLOW)
      << "\tUnderFlow exception is" << on_or_off(femask & FE_UNDERFLOW)
      ;
  }
#endif
}

// ======================================================================
PROVIDE_FILE_PATH()
PROVIDE_ALLOWED_CONFIGURATION(art::FloatingPointControl)
