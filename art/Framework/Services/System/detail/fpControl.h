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
#define fpControl_ALL_PREC fpControl_EXTENDED_PREC | fpControl_DOUBLE_PREC | fpControl_SINGLE_PREC
#ifdef __x86_64__
#define fpControl_HAVE_MXCSR
#endif
#else
#error Architecture not valid for FP control
#endif
}

namespace art {
  namespace detail {

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

    fpcw_t getFPCW();
    fpcw_t setFPCW(fpcw_t fpcw);

#ifdef fpControl_HAVE_MXCSR
    mxcsr_t getMXCSR();
    mxcsr_t setMXCSR(mxcsr_t mxcsr);
#endif

    fp_control_t getFPControl();
    fp_control_t setFPControl(fp_control_t const & fpControl);
  }

  inline
  detail::fp_control_t
  detail::getFPControl()
  {
    fp_control_t result { getFPCW()
#ifdef fpControl_HAVE_MXCSR
        , getMXCSR()
#endif
        };
    return result;
  }

  inline
  detail::fp_control_t
  detail::setFPControl(fp_control_t const & fpControl)
  {
    fp_control_t result { setFPCW(fpControl.fpcw)
#ifdef fpControl_HAVE_MXCSR
        , setMXCSR(fpControl.mxcsr)
#endif
        };
    return result;
  }
}

#endif /* art_Framework_Services_System_detail_fpControl_h */

// Local Variables:
// mode: c++
// End:
