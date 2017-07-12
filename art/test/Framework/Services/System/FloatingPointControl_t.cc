#include "catch/catch.hpp"

#include "art/Framework/Services/System/FloatingPointControl.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "messagefacility/plugins/stringstream.h"

extern "C" {
#include <fenv.h>
#include "xpfpa/xpfpa.h"
}

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

using namespace std::string_literals;

namespace {
  auto & fpcw(fenv_t & fe)
  {
#if defined __i386__ || defined __x86_64__
#if defined __APPLE__
    return fe.__control;
#elif defined __linux__
    return fe.__control_word;
#else
#error OS not valid for FP control
#endif
#else
#error Architecture not valid for FP control
#endif
  }

  auto fpcw_snapshot() {
    fenv_t fe;
    fegetenv(&fe);
    return fpcw(fe);
  }
  
  static auto const fpcw_ref = fpcw_snapshot();

  static decltype(fpcw_ref) ART_FP_SINGLE_PREC =
#if defined HAVE__CONTROLFP_S || defined HAVE__CONTROLFP
    _PC_24
#elif defined HAVE__FPU_SETCW
    _FPU_SINGLE
#elif defined HAVE_FPU_INLINE_ASM_X86
    0x000
#endif
    ;
  
  static decltype(fpcw_ref) ART_FP_DOUBLE_PREC =
#if defined HAVE__CONTROLFP_S || defined HAVE__CONTROLFP
    _PC_53
#elif defined HAVE__FPU_SETCW
    _FPU_DOUBLE
#elif defined HAVE_FPU_INLINE_ASM_X86
    0x200
#endif
    ;
  
  static decltype(fpcw_ref) ART_FP_EXTENDED_PREC =
#if defined HAVE__CONTROLFP_S || defined HAVE__CONTROLFP
    _PC_64
#elif defined HAVE__FPU_SETCW
    _FPU_EXTENDED
#elif defined HAVE_FPU_INLINE_ASM_X86
    0x300
#endif
    ;
  
  static decltype(fpcw_ref) ART_FP_ALL =
    ART_FP_SINGLE_PREC | ART_FP_DOUBLE_PREC | ART_FP_EXTENDED_PREC;

  auto const fpcw_ref_dp = (fpcw_ref & ~ ART_FP_ALL) | ART_FP_DOUBLE_PREC;
  
  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
  class Hexable {
public:
    Hexable(T const t) : t_(t) { }
    operator T () const { return t_; }
private:
    T t_;
  };

  template <typename T>
  inline
  Hexable<T> hexit(T const t)
  { return Hexable<T>(t); }

  template <typename T>
  inline
  void
  compare_equal(T const left,
                decltype(left) right,
                std::string msg = "Checking current FP state against expected"s)
  {
    INFO(msg);
    CHECK(hexit(left) == hexit(right));
  }

  char const* on_or_off (bool const b)
  {
    return b ? " on " : " off";
  }

  auto const fmt = "DivByZero exception is%4s"
    "\tInvalid exception is%4s"
    "\tOverFlow exception is%4s"
    "\tUnderFlow exception is%4s\n";

  template <typename INT, typename = std::enable_if_t<std::is_integral<INT>::value>>
  void
  verify_report(std::ostringstream const & os, INT mask) {
    auto test_string = os.str();
    test_string = test_string.substr(test_string.rfind("DivByZero exception is "));
    std::string ref(test_string.size(), 0);
    snprintf(&*ref.begin(), ref.size() + 1, fmt,
             on_or_off((~ mask) & FE_DIVBYZERO),
             on_or_off((~ mask) & FE_INVALID),
             on_or_off((~ mask) & FE_OVERFLOW),
             on_or_off((~ mask) & FE_UNDERFLOW));
    CHECK(test_string == ref);
  }

  void
  verify_report(std::ostringstream const & os) {
    verify_report(os, (decltype(FE_ALL_EXCEPT)) 0);
  }

  void
  verify_cleanup(art::ActivityRegistry & reg)
  {
      reg.sPostEndJob.invoke();
      compare_equal(fpcw_snapshot(),
                    fpcw_ref,
                    "Checking final FP state against default"s);
  }
}

namespace Catch {
  template <typename T> struct StringMaker<Hexable<T>> {
    static std::string convert(T const t)
      {
        std::string result;
        std::ostringstream out;
        out << "0x" << std::hex << t;
        result = out.str();
        return result;
      }
  };
}

SCENARIO("We wish to affect the floating point control on our system")
{
  GIVEN("An empty activity registry and simple parameter set") {
    art::ActivityRegistry reg;

    fhicl::ParameterSet ps;
    ps.put("reportSettings", true);

    // Note config above sends output to stream we can capture:
    auto & tstream = mf::getStringStream("TSTREAM_1");
    tstream.str("");

    WHEN("We want the basic configuration")
    {
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration is unchanged except for double precision math.")
      {
        compare_equal(fpcw_snapshot(), fpcw_ref_dp);
        verify_report(tstream);
        verify_cleanup(reg);
      }
    }

    WHEN("We want to suppress divide-by-zero exceptions")
    {
      ps.put("enableDivByZeroEx", true);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows suppressed divide-by-zero exceptions")
      {
        compare_equal(fpcw_snapshot(), fpcw_ref_dp & ~ FE_DIVBYZERO);
        verify_report(tstream, FE_DIVBYZERO);
        verify_cleanup(reg);
      }
    }

    WHEN("We want to suppress \"invalid\" exceptions")
    {
      ps.put("enableInvalidEx", true);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows suppressed \"invalid\" exceptions")
      {
        compare_equal(fpcw_snapshot(), fpcw_ref_dp & ~ FE_INVALID);
        verify_report(tstream, FE_INVALID);
        verify_cleanup(reg);
      }
    }
  
    WHEN("We want to suppress overflow exceptions")
    {
      ps.put("enableOverFlowEx", true);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows suppressed overflow exceptions")
      {
        compare_equal(fpcw_snapshot(), fpcw_ref_dp & ~ FE_OVERFLOW);
        verify_report(tstream, FE_OVERFLOW);
        verify_cleanup(reg);
      }
    }

    WHEN("We want to suppress underflow exceptions")
    {
      ps.put("enableUnderFlowEx", true);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows suppressed underflow exceptions")
      {
        compare_equal(fpcw_snapshot(), fpcw_ref_dp & ~ FE_UNDERFLOW);
        verify_report(tstream, FE_UNDERFLOW);
        verify_cleanup(reg);
      }
    }

    WHEN("We want default precision")
    {
      ps.put("setPrecisionDouble", false);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows default precision")
      {
        compare_equal(fpcw_snapshot(), fpcw_ref);
        verify_report(tstream);
        verify_cleanup(reg);
      }
    }
  }
}
