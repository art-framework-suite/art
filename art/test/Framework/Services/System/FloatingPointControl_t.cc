#include "catch2/catch.hpp"

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/System/FloatingPointControl.h"
#include "art/Framework/Services/System/detail/fpControl.h"
#include "messagefacility/plugins/stringstream.h"

#include <sstream>
#include <string>

#include <type_traits>

using namespace std::string_literals;
using namespace art::fp_detail;

namespace {
  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
  class Hexable {
  public:
    Hexable(T const t) : t_(t) {}
    operator T() const { return t_; }

  private:
    T t_;
  };

  template <typename T>
  inline Hexable<T>
  hexit(T const t)
  {
    return Hexable<T>(t);
  }

  inline void
  compare_fpcw(art::FloatingPointControl& fpc,
               fpcw_t const right,
               std::string msg = "Checking current FP state against expected"s)
  {
    INFO(msg);
    CHECK(hexit(getFPCW()) == hexit(right));
    CHECK(hexit(static_cast<unsigned short int>(fpc.getPrecision())) ==
          hexit(right & fpControl_ALL_PREC));
    CHECK(hexit(fpc.getMask()) == hexit(right & FE_ALL_EXCEPT));
  }

  void
  verify_report(std::ostringstream const& os, fpcw_t mask)
  {
    auto test_string = os.str();
    auto const pos = test_string.rfind("DivByZero exception is ");
    if (pos != std::string::npos) {
      std::string const ref(
        "DivByZero exception is"s + on_or_off((~mask) & FE_DIVBYZERO) +
        "\tInvalid exception is" + on_or_off((~mask) & FE_INVALID) +
        "\tOverFlow exception is" + on_or_off((~mask) & FE_OVERFLOW) +
        "\tUnderFlow exception is" + on_or_off((~mask) & FE_UNDERFLOW) + '\n');
      CHECK(test_string.substr(pos) == ref);
    } else {
      CHECK(false);
    }
  }

  void
  verify_report(std::ostringstream const& os)
  {
    verify_report(os, 0);
  }

} // namespace

namespace Catch {
  template <typename T>
  struct StringMaker<Hexable<T>> {
    static std::string
    convert(T const t)
    {
      std::string result;
      std::ostringstream out;
      out << std::showbase << std::hex << t;
      result = out.str();
      return result;
    }
  };
} // namespace Catch

SCENARIO("We wish to affect the floating point control on our system")
{
  GIVEN("An empty activity registry and simple parameter set")
  {
    art::ActivityRegistry reg;

    fhicl::ParameterSet ps;
    ps.put("reportSettings", true);

    // Note config above sends output to stream we can capture:
    auto& tstream = mf::getStringStream("TSTREAM_1");
    tstream.str("");

    auto const fpcw_def = getFPCW();
    auto const fpcw_ref = fpcw_def;
    auto const fpcw_ref_dp =
      (fpcw_ref & ~fpControl_ALL_PREC) | fpControl_DOUBLE_PREC;

    static auto verify_cleanup = [fpcw_def, &reg](auto& fpc) {
      reg.sPostEndJob.invoke();
      compare_fpcw(fpc, fpcw_def, "Checking final FP state against default"s);
    };

    WHEN("We want the basic configuration")
    {
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration is unchanged except for double precision math.")
      {
        compare_fpcw(fpc, fpcw_ref_dp);
        verify_report(tstream);
        verify_cleanup(fpc);
      }
    }

    WHEN("We want to suppress divide-by-zero exceptions")
    {
      ps.put("enableDivByZeroEx", true);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows suppressed divide-by-zero exceptions")
      {
        compare_fpcw(fpc, fpcw_ref_dp & ~FE_DIVBYZERO);
        verify_report(tstream, FE_DIVBYZERO);
        verify_cleanup(fpc);
      }
    }

    WHEN("We want to suppress \"invalid\" exceptions")
    {
      ps.put("enableInvalidEx", true);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows suppressed \"invalid\" exceptions")
      {
        compare_fpcw(fpc, fpcw_ref_dp & ~FE_INVALID);
        verify_report(tstream, FE_INVALID);
        verify_cleanup(fpc);
      }
    }

    WHEN("We want to suppress overflow exceptions")
    {
      ps.put("enableOverFlowEx", true);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows suppressed overflow exceptions")
      {
        compare_fpcw(fpc, fpcw_ref_dp & ~FE_OVERFLOW);
        verify_report(tstream, FE_OVERFLOW);
        verify_cleanup(fpc);
      }
    }

    WHEN("We want to suppress underflow exceptions")
    {
      ps.put("enableUnderFlowEx", true);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows suppressed underflow exceptions")
      {
        compare_fpcw(fpc, fpcw_ref_dp & ~FE_UNDERFLOW);
        verify_report(tstream, FE_UNDERFLOW);
        verify_cleanup(fpc);
      }
    }

    WHEN("We want default precision")
    {
      ps.put("setPrecisionDouble", false);
      art::FloatingPointControl fpc(ps, reg);
      THEN("The configuration shows default precision")
      {
        compare_fpcw(fpc, fpcw_ref);
        verify_report(tstream);
        verify_cleanup(fpc);
      }
    }
  }
}
