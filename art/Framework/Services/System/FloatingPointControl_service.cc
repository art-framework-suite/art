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

namespace {
  char const* on_or_off (bool const b)
  {
    return b ? " on " : " off";
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
  , OSdefault_(detail::getFPControl())
{
  reg.sPostEndJob.watch(this, &FloatingPointControl::postEndJob);

  if (reportSettings_) {
    mf::LogVerbatim("FPE_Enable") << "\nOS's FP settings";
    echoState();
  }

  // Update the state according to the user's configuration
  controlFpe();
  if (reportSettings_) {
    mf::LogVerbatim("FPE_Enable") << "\nUpdated FP settings per user's configuration";
    echoState();
  }
}

// ----------------------------------------------------------------------

void
art::FloatingPointControl::postEndJob()
{
  (void) detail::setFPControl(OSdefault_);
  if (reportSettings_) {
    mf::LogVerbatim("FPE_Enable") << "\nRestored to OS's FPE settings";
    echoState();
  }
}

// ----------------------------------------------------------------------

void
art::FloatingPointControl::controlFpe()
{
  // NB: We do not let users configure signaling of denormalized operand
  // (fpControl_DENORMALOPERAND) or inexact (FE_INEXACT), both of which
  // are suppressed.
  unsigned short int enable_except {};
  unsigned short int enable_sse {};

  if (enableDivByZeroEx_) {
    enable_except |= FE_DIVBYZERO;
    enable_sse |= ART_ZM_MASK;
  }
  
  if (enableInvalidEx_) {
    enable_except |= FE_INVALID;
    enable_sse |= ART_IM_MASK;
  }
  
  if (enableOverFlowEx_) {
    enable_except |= FE_OVERFLOW;
    enable_sse |= ART_OM_MASK;
  }
  
  if (enableUnderFlowEx_) {
    enable_except |= FE_UNDERFLOW;
    enable_sse |= ART_UM_MASK;
  }

  auto fpControl = detail::getFPControl();

  fpControl.fpcw &= ~enable_except;

  if (setPrecisionDouble_) {
    // Set precision.
    fpControl.fpcw = (fpControl.fpcw &~ (fpControl_ALL_PREC)) | fpControl_DOUBLE_PREC;
  }

#ifdef fpControl_HAVE_MXCSR
  // Also set the bits in the SSE unit.
  fpControl.mxcsr &= ~enable_sse;
#endif

  // Write back.
  (void) detail::setFPControl(fpControl);
}

// ----------------------------------------------------------------------

void
art::FloatingPointControl::echoState()
{
  if (reportSettings_) {
    auto const femask { detail::getFPCW() };
    mf::LogVerbatim("FPE_Enable")
      << "Floating point exception mask is "
      << std::showbase << std::hex << femask
      << "\tDivByZero exception is" << on_or_off(femask & FE_DIVBYZERO)
      << "\tInvalid exception is"   << on_or_off(femask & FE_INVALID)
      << "\tOverFlow exception is"  << on_or_off(femask & FE_OVERFLOW)
      << "\tUnderFlow exception is" << on_or_off(femask & FE_UNDERFLOW)
      ;
  }
}

// ======================================================================
PROVIDE_FILE_PATH()
PROVIDE_ALLOWED_CONFIGURATION(art::FloatingPointControl)
