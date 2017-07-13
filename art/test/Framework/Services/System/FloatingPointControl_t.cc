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

  static auto constexpr ART_FP_SINGLE_PREC =
#if defined HAVE__CONTROLFP_S || defined HAVE__CONTROLFP
    _PC_24
#elif defined HAVE__FPU_SETCW
    _FPU_SINGLE
#elif defined HAVE_FPU_INLINE_ASM_X86
    0x000
#endif
    ;
  
  static auto constexpr ART_FP_DOUBLE_PREC =
#if defined HAVE__CONTROLFP_S || defined HAVE__CONTROLFP
    _PC_53
#elif defined HAVE__FPU_SETCW
    _FPU_DOUBLE
#elif defined HAVE_FPU_INLINE_ASM_X86
    0x200
#endif
    ;
  
  static auto constexpr ART_FP_EXTENDED_PREC =
#if defined HAVE__CONTROLFP_S || defined HAVE__CONTROLFP
    _PC_64
#elif defined HAVE__FPU_SETCW
    _FPU_EXTENDED
#elif defined HAVE_FPU_INLINE_ASM_X86
    0x300
#endif
    ;
  
  static auto constexpr ART_FP_ALL =
    ART_FP_SINGLE_PREC | ART_FP_DOUBLE_PREC | ART_FP_EXTENDED_PREC;

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
  compare_fpcw(T const right,
               std::string msg = "Checking current FP state against expected"s)
  {
    INFO(msg);
    fenv_t fe;
    fegetenv(&fe);
    auto const left = fpcw(fe);
    CHECK(hexit(left) == hexit(right));
  }

  char const* on_or_off (bool const b)
  {
    return b ? " on " : " off";
  }

  template <typename INT, typename = std::enable_if_t<std::is_integral<INT>::value>>
  void
  verify_report(std::ostringstream const & os, INT mask) {
    auto test_string = os.str();
    auto const pos = test_string.rfind("DivByZero exception is ");
    if (pos != std::string::npos) {
      std::string const ref("DivByZero exception is"s +
                            on_or_off((~ mask) & FE_DIVBYZERO) +
                            "\tInvalid exception is" +
                            on_or_off((~ mask) & FE_INVALID) +
                            "\tOverFlow exception is" +
                            on_or_off((~ mask) & FE_OVERFLOW) +
                            "\tUnderFlow exception is" +
                            on_or_off((~ mask) & FE_UNDERFLOW) + '\n');
      CHECK(test_string.substr(pos) == ref);
    } else {
      CHECK(false);
    }
  }

  void
  verify_report(std::ostringstream const & os) {
    verify_report(os, (decltype(FE_ALL_EXCEPT)) 0);
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

    fenv_t fenv;
    fegetenv(&fenv);
    auto const fpcw_ref = fpcw(fenv);
    auto const fpcw_ref_dp = (fpcw_ref & ~ ART_FP_ALL) | ART_FP_DOUBLE_PREC;

    static auto verify_cleanup =
      [fpcw_ref, &reg]()
      {
        reg.sPostEndJob.invoke();
        compare_fpcw(fpcw_ref, "Checking final FP state against default"s);
      };

    WHEN("We want the basic configuration")
    {
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration is unchanged except for double precision math.")
      {
        compare_fpcw(fpcw_ref_dp);
        verify_report(tstream);
        verify_cleanup();
      }
    }

    WHEN("We want to suppress divide-by-zero exceptions")
    {
      ps.put("enableDivByZeroEx", true);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows suppressed divide-by-zero exceptions")
      {
        compare_fpcw(fpcw_ref_dp & ~ FE_DIVBYZERO);
        verify_report(tstream, FE_DIVBYZERO);
        verify_cleanup();
      }
    }

    WHEN("We want to suppress \"invalid\" exceptions")
    {
      ps.put("enableInvalidEx", true);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows suppressed \"invalid\" exceptions")
      {
        compare_fpcw(fpcw_ref_dp & ~ FE_INVALID);
        verify_report(tstream, FE_INVALID);
        verify_cleanup();
      }
    }
  
    WHEN("We want to suppress overflow exceptions")
    {
      ps.put("enableOverFlowEx", true);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows suppressed overflow exceptions")
      {
        compare_fpcw(fpcw_ref_dp & ~ FE_OVERFLOW);
        verify_report(tstream, FE_OVERFLOW);
        verify_cleanup();
      }
    }

    WHEN("We want to suppress underflow exceptions")
    {
      ps.put("enableUnderFlowEx", true);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows suppressed underflow exceptions")
      {
        compare_fpcw(fpcw_ref_dp & ~ FE_UNDERFLOW);
        verify_report(tstream, FE_UNDERFLOW);
        verify_cleanup();
      }
    }

    WHEN("We want default precision")
    {
      ps.put("setPrecisionDouble", false);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows default precision")
      {
        compare_fpcw(fpcw_ref);
        verify_report(tstream);
        verify_cleanup();
      }
    }
  }
}
