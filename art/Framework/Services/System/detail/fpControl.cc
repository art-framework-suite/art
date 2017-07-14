#include "art/Framework/Services/System/detail/fpControl.h"

#if defined __i386__ || defined __x86_64__
#ifdef __linux__
#include <fpu_control.h>
#define fpControl_GETFPCW _FPU_GETCW
#define fpControl_SETFPCW _FPU_SETCW
#elif defined __APPLE__
#define fpControl_GETFPCW(cw) __asm__ __volatile__ ("fnstcw %0" : "=m" (*&cw))
#define fpControl_SETFPCW(cw) __asm__ __volatile__ ("fldcw %0" : : "m" (*&cw))
#else
#error OS not valid for FP control
#endif
#ifdef fpControl_HAVE_MXCSR
#define fpControl_GETMXCSR(cw) __asm__ __volatile__ ("stmxcsr %0" : "=m" (*&cw));
#define fpControl_SETMXCSR(cw) __asm__ __volatile__ ("ldmxcsr %0" : : "m" (*&cw));
#endif
#endif

art::detail::fpcw_t
art::detail::getFPCW()
{
  fpcw_t result;
  fpControl_GETFPCW(result);
  return result;
}

art::detail::fpcw_t
art::detail::setFPCW(fpcw_t const fpcw)
{
  fpcw_t result = getFPCW();
  fpControl_SETFPCW(fpcw);
  return result;
}

#ifdef fpControl_HAVE_MXCSR
art::detail::mxcsr_t
art::detail::getMXCSR()
{
  mxcsr_t result;
  fpControl_GETMXCSR(result);
  return result;
}

art::detail::mxcsr_t
art::detail::setMXCSR(mxcsr_t const mxcsr)
{
  mxcsr_t result = getMXCSR();
  fpControl_SETMXCSR(mxcsr);
  return result;
}

#endif
