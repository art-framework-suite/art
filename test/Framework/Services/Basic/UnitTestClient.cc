#include "test/Framework/Services/Basic/UnitTestClient.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>

#include <cmath>
#include <float.h>

namespace arttest
{
void
  UnitTestClient::analyze( art::Event      const & e
                           , art::EventSetup const & es )
{

  double x = 1.0;
  double y = DBL_MAX;

  if(e.id().event() == 2) {
    art::LogVerbatim("FPExceptions") << "\n\t\tx = " << x;
    art::LogVerbatim("FPExceptions") << "\t\ty = " << y << " (DBL_MAX)";

  // DivideByZero
    art::LogVerbatim("FPExceptions") << "\t\tForce DivideByZero: a = x/zero";
    double zero = 0.0;
    double a = x / zero;
    art::LogVerbatim("FPExceptions") << "\t\ta = " << a;

  // Invalid
    art::LogVerbatim("FPExceptions") << "\t\tForce Invalid: b = std::log(-1.0)";
    double b = std::log(-1.0);
    art::LogVerbatim("FPExceptions") << "\t\tb = " << b;

  // Overflow (actually precision)
    art::LogVerbatim("FPExceptions") << "\t\tForce Overflow: c = y*y";
    double c = y * y;
    art::LogVerbatim("FPExceptions") << "\t\tc = " << c;

  // Underflow (actually precision)
    art::LogVerbatim("FPExceptions") << "\t\tForce Underflow: d = x/y";
    double d = x / y;
    art::LogVerbatim("FPExceptions") << "\t\td = " << d;
  }
}

}  // arttest


using arttest::UnitTestClient;
DEFINE_FWK_MODULE(UnitTestClient);
