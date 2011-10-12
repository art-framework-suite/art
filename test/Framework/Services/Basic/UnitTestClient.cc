#include "test/Framework/Services/Basic/UnitTestClient.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "cpp0x/cmath"
#include <float.h>
#include <iostream>
#include <string>

namespace arttest {
  void
  UnitTestClient::analyze(art::Event      const & e
                          , art::EventSetup const & es)
  {
    double x = 1.0;
    double y = DBL_MAX;
    if (e.id().event() == 2) {
      mf::LogVerbatim("FPExceptions") << "\n\t\tx = " << x;
      mf::LogVerbatim("FPExceptions") << "\t\ty = " << y << " (DBL_MAX)";
      // DivideByZero
      mf::LogVerbatim("FPExceptions") << "\t\tForce DivideByZero: a = x/zero";
      double zero = 0.0;
      double a = x / zero;
      mf::LogVerbatim("FPExceptions") << "\t\ta = " << a;
      // Invalid
      mf::LogVerbatim("FPExceptions") << "\t\tForce Invalid: b = std::log(-1.0)";
      double b = std::log(-1.0);
      mf::LogVerbatim("FPExceptions") << "\t\tb = " << b;
      // Overflow (actually precision)
      mf::LogVerbatim("FPExceptions") << "\t\tForce Overflow: c = y*y";
      double c = y * y;
      mf::LogVerbatim("FPExceptions") << "\t\tc = " << c;
      // Underflow (actually precision)
      mf::LogVerbatim("FPExceptions") << "\t\tForce Underflow: d = x/y";
      double d = x / y;
      mf::LogVerbatim("FPExceptions") << "\t\td = " << d;
    }
  }

}  // arttest


using arttest::UnitTestClient;
DEFINE_ART_MODULE(UnitTestClient);
