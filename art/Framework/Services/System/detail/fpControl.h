#ifndef art_Framework_Services_System_detail_fpControl_h
#define art_Framework_Services_System_detail_fpControl_h

extern "C" {
#if defined __i386__ || defined __x86_64__
#include <fenv.h>
#ifdef __linux__
#define fpControl_DENORMALOPERAND __FE_DENORM
#include <fpu_control.h>
#define fpControl_EXTENDED_PREC _FPU_EXTENDED
#define fpControl_DOUBLE_PREC _FPU_DOUBLE
#define fpControl_SINGLE_PREC _FPU_SINGLE
#elif defined __APPLE__
#ifndef __GNUC__
#pragma STDC FENV_ACCESS ON
#endif
#define fpControl_DENORMALOPERAND FE_DENORMALOPERAND
#define fpControl_EXTENDED_PREC 0x300
#define fpControl_DOUBLE_PREC 0x200
#define fpControl_SINGLE_PREC 0x0
#else
#error OS not valid for FP control
#endif
#define fpControl_ALL_PREC (fpControl_EXTENDED_PREC | fpControl_DOUBLE_PREC | fpControl_SINGLE_PREC)
#ifdef __x86_64__
#define fpControl_HAVE_MXCSR
// for the MMU (SSE), here are the bit definitions
// Pnemonic Bit Location Description
// FZ bit 15 Flush To Zero
#define fpControl_FZ 0x8000
// R+ bit 14 Round Positive
#define fpControl_R_PLUS 0x4000
// R- bit 13 Round Negative
#define fpControl_R_MINUS 0x2000
// RZ bits 13 and 14 Round To Zero
#define fpControl_RZ (fpControl_R_PLUS|fpControl_R_MINUS)
// RN bits 13 and 14 are 0 Round To Nearest
#define fpControl_RN_MASK fpControl_RZ
// PM bit 12 Precision Mask
#define fpControl_PM_MASK (0x1000)
// UM bit 11 Underflow Mask
#define fpControl_UM_MASK (0x0800)
// OM bit 10 Overflow Mask
#define fpControl_OM_MASK (0x0400)
// ZM bit 9 Divide By Zero Mask
#define fpControl_ZM_MASK (0x0200)
// DM bit 8 Denormal Mask
#define fpControl_DM_MASK (0x0100)
// IM bit 7 Invalid Operation Mask
#define fpControl_IM_MASK (0x0080)
// DAZ bit 6 Denormals Are Zero
#define fpControl_DAX 0x0040
// PE bit 5 Precision Flag
#define fpControl_PE 0x0020
// UE bit 4 Underflow Flag
#define fpControl_UE 0x0010
// OE bit 3 Overflow Flag
#define fpControl_OE 0x0008
// ZE bit 2 Divide By Zero Flag
#define fpControl_ZE 0x0004
// DE bit 1 Denormal Flag
#define fpControl_DE 0x0002
// IE bit 0 Invalid Operation Flag
#define fpControl_IE 0x0001
// All mask bits
#define fpControl_ALL_SSE_EXCEPT 0x1f80
// All flag bits
#define fpControl_ALL_SSE_FLAGS 0x3f
#endif /* __x86_64__ */
#else
#error Architecture not valid for FP control
#endif
}

namespace art {
  namespace fp_detail {

    enum class precision_t {
      SINGLE = fpControl_SINGLE_PREC,
      DOUBLE = fpControl_DOUBLE_PREC,
      EXTENDED = fpControl_EXTENDED_PREC
    };

    using fpsw_t =
#ifdef __linux__
      decltype(fenv_t::__status_word)
#elif __APPLE__
      decltype(fenv_t::__status)
#endif
      ;

    using fpcw_t =
#ifdef __linux__
      decltype(fenv_t::__control_word)
#elif __APPLE__
      decltype(fenv_t::__control)
#endif
      ;

#ifdef fpControl_HAVE_MXCSR
    using mxcsr_t = decltype(fenv_t::__mxcsr);
#endif

    struct fp_control_t {
      fpcw_t fpcw;
#ifdef fpControl_HAVE_MXCSR
      mxcsr_t mxcsr; // SSE
#endif
    };

    fpsw_t getFPSW();

    fpcw_t getFPCW();
    fpcw_t setFPCW(fpcw_t fpcw);

#ifdef fpControl_HAVE_MXCSR
    mxcsr_t getMXCSR();
    mxcsr_t setMXCSR(mxcsr_t mxcsr);
#endif

    fp_control_t getFPControl();
    fp_control_t setFPControl(fp_control_t const & fpControl);

    char const* on_or_off (bool const b);
  }
}

inline
art::fp_detail::fp_control_t
art::fp_detail::getFPControl()
{
  fp_control_t result { getFPCW()
#ifdef fpControl_HAVE_MXCSR
      , getMXCSR()
#endif
      };
  return result;
}

inline
art::fp_detail::fp_control_t
art::fp_detail::setFPControl(fp_control_t const & fpControl)
{
  fp_control_t result { setFPCW(fpControl.fpcw)
#ifdef fpControl_HAVE_MXCSR
      , setMXCSR(fpControl.mxcsr)
#endif
      };
  return result;
}

inline
char const*
art::fp_detail::on_or_off (bool const b)
{
  return b ? " on " : " off";
}
#endif /* art_Framework_Services_System_detail_fpControl_h */

// Local Variables:
// mode: c++
// End:
