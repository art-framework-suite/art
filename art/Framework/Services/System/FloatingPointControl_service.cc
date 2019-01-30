#include "art/Framework/Services/System/FloatingPointControl.h"
// vim: set sw=2 expandtab :

#include "cetlib/ProvideFilePathMacro.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/AllowedConfigurationMacro.h"

using namespace std;
using namespace art::fp_detail;

namespace art {

  namespace {

    void
    echoState()
    {
      auto const femask{getFPCW()};
      mf::LogVerbatim("FPE_Enable")
        << "Floating point exception mask is " << std::showbase << std::hex
        << femask << "\tDivByZero exception is"
        << on_or_off(femask & FE_DIVBYZERO) << "\tInvalid exception is"
        << on_or_off(femask & FE_INVALID) << "\tOverFlow exception is"
        << on_or_off(femask & FE_OVERFLOW) << "\tUnderFlow exception is"
        << on_or_off(femask & FE_UNDERFLOW);
    }

  } // unnamed namespacde

  FloatingPointControl::FloatingPointControl(Parameters const& c,
                                             ActivityRegistry& reg)
    : enableDivByZeroEx_{c().enableDivByZeroEx()}
    , enableInvalidEx_{c().enableInvalidEx()}
    , enableOverFlowEx_{c().enableOverFlowEx()}
    , enableUnderFlowEx_{c().enableUnderFlowEx()}
    , setPrecisionDouble_{c().setPrecisionDouble()}
    , reportSettings_{c().reportSettings()}
    , OSdefault_(getFPControl())
  {
    reg.sPostEndJob.watch(this, &FloatingPointControl::postEndJob);
    if (reportSettings_) {
      mf::LogVerbatim("FPE_Enable") << "\nOS's FP settings";
      echoState();
    }
    // Update the state according to the user's configuration
    {
      // NB: We do not let users configure signaling of denormalized operand
      // (fpControl_DENORMALOPERAND) or inexact (FE_INEXACT), both of which
      // are suppressed.
      unsigned short int enable_except{};
      unsigned short int enable_sse{};
      if (enableDivByZeroEx_) {
        enable_except |= FE_DIVBYZERO;
        enable_sse |= fpControl_ZM_MASK;
      }
      if (enableInvalidEx_) {
        enable_except |= FE_INVALID;
        enable_sse |= fpControl_IM_MASK;
      }
      if (enableOverFlowEx_) {
        enable_except |= FE_OVERFLOW;
        enable_sse |= fpControl_OM_MASK;
      }
      if (enableUnderFlowEx_) {
        enable_except |= FE_UNDERFLOW;
        enable_sse |= fpControl_UM_MASK;
      }
      auto fpControl = getFPControl();
      // Reset exception mask before clearing the bits we care about.
      fpControl.fpcw = (fpControl.fpcw | FE_ALL_EXCEPT) & (~enable_except);
      if (setPrecisionDouble_) {
        // Clear precision bits before setting only the ones we care about.
        fpControl.fpcw =
          (fpControl.fpcw & (~fpControl_ALL_PREC)) | fpControl_DOUBLE_PREC;
      }
#ifdef fpControl_HAVE_MXCSR
      // Reset exception mask before clearing the bits we care about.
      fpControl.mxcsr =
        (fpControl.mxcsr | fpControl_ALL_SSE_EXCEPT) & (~enable_sse);
#endif
      // Write back.
      (void)setFPControl(fpControl);
    }
    if (reportSettings_) {
      mf::LogVerbatim("FPE_Enable")
        << "\nUpdated FP settings per user's configuration";
      if (reportSettings_) {
        echoState();
      }
    }
  }

  void
  FloatingPointControl::postEndJob()
  {
    (void)setFPControl(OSdefault_);
    if (reportSettings_) {
      mf::LogVerbatim("FPE_Enable") << "\nRestored to OS's FPE settings";
      echoState();
    }
  }

  auto
  FloatingPointControl::getPrecision() const -> precision_t
  {
    return static_cast<precision_t>(getFPCW() & fpControl_ALL_PREC);
  }

  auto
  FloatingPointControl::getMask() const -> mask_t
  {
    return static_cast<mask_t>(getFPCW() & FE_ALL_EXCEPT);
  }

} // namespace art

CET_PROVIDE_FILE_PATH()
FHICL_PROVIDE_ALLOWED_CONFIGURATION(art::FloatingPointControl)
