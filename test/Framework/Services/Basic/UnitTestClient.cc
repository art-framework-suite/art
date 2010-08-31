#include "test/Framework/Services/Basic/UnitTestClient.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>

#include <cmath>
#include <float.h>

namespace edmtest
{
void
  UnitTestClient::analyze( edm::Event      const & e
                           , edm::EventSetup const & es )
{

  double x = 1.0;
  double y = DBL_MAX;

  if(e.id().event() == 2) {
    edm::LogVerbatim("FPExceptions") << "\n\t\tx = " << x;
    edm::LogVerbatim("FPExceptions") << "\t\ty = " << y << " (DBL_MAX)";

  // DivideByZero
    edm::LogVerbatim("FPExceptions") << "\t\tForce DivideByZero: a = x/zero";
    double zero = 0.0;
    double a = x / zero;
    edm::LogVerbatim("FPExceptions") << "\t\ta = " << a;

  // Invalid
    edm::LogVerbatim("FPExceptions") << "\t\tForce Invalid: b = std::log(-1.0)";
    double b = std::log(-1.0);
    edm::LogVerbatim("FPExceptions") << "\t\tb = " << b;

  // Overflow (actually precision)
    edm::LogVerbatim("FPExceptions") << "\t\tForce Overflow: c = y*y";
    double c = y * y;
    edm::LogVerbatim("FPExceptions") << "\t\tc = " << c;

  // Underflow (actually precision)
    edm::LogVerbatim("FPExceptions") << "\t\tForce Underflow: d = x/y";
    double d = x / y;
    edm::LogVerbatim("FPExceptions") << "\t\td = " << d;
  }
}

}  // namespace edmtest


using edmtest::UnitTestClient;
DEFINE_FWK_MODULE(UnitTestClient);
