// vim: set sw=2 expandtab :

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/test/Framework/Services/Basic/fpc_utils.h"
#include "hep_concurrency/tsan.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cmath>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include <float.h>
#include <memory.h>

using namespace std;
using namespace art;

extern "C" {
void
handle_sigfpe(int, siginfo_t*, void*)
{
  cerr << "Worked!\n";
  exit(0);
}
} // extern "C"

namespace arttest {

  class FPCTest : public EDAnalyzer {
  public:
    explicit FPCTest(fhicl::ParameterSet const&);

  private:
    void analyze(Event const&);
  };

  FPCTest::FPCTest(fhicl::ParameterSet const& p) : EDAnalyzer{p}
  {
    // Install the SIGFPE handler.
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = handle_sigfpe;
    act.sa_flags = SA_RESTART;
    if (sigaction(SIGFPE, &act, 0) != 0) {
      perror("sigaction failed");
      throw runtime_error("cannot install sigaction signal handler");
    }
    // Block the floating point exception signal SIGFPE.
    // We need this to be done in the main thread early
    // so that it is blocked in all threads started by tbb.
    sigset_t newset;
    sigemptyset(&newset);
    sigaddset(&newset, SIGFPE);
    pthread_sigmask(SIG_BLOCK, &newset, 0);
  }

  void
  FPCTest::analyze(Event const& e)
  {
    double const x{1.0};
    double const y{DBL_MAX};
    if (e.id().event() == 2) {
      // Allow the floating point exception signal SIGFPE.
      sigset_t newset;
      sigemptyset(&newset);
      sigaddset(&newset, SIGFPE);
      pthread_sigmask(SIG_UNBLOCK, &newset, 0);
      // Show the variable values.
      mf::LogVerbatim("FPExceptions") << "\n\t\tx = " << x;
      mf::LogVerbatim("FPExceptions") << "\t\ty = " << y << " (DBL_MAX)";
      // DivideByZero
      mf::LogVerbatim("FPExceptions") << "\t\tForce DivideByZero: a = x/zero";
      double const a{divit(x, 0)};
      mf::LogVerbatim("FPExceptions") << "\t\ta = " << a;
      // Invalid
      mf::LogVerbatim("FPExceptions") << "\t\tForce Invalid: b = log(-1.0)";
      double const b{log(-1.0)};
      mf::LogVerbatim("FPExceptions") << "\t\tb = " << b;
      // Overflow (actually precision)
      mf::LogVerbatim("FPExceptions") << "\t\tForce Overflow: c = y*y";
      double const c{multit(y, y)};
      mf::LogVerbatim("FPExceptions") << "\t\tc = " << c;
      // Underflow (actually precision)
      mf::LogVerbatim("FPExceptions") << "\t\tForce Underflow: d = x/y";
      double const d{divit(x, y)};
      mf::LogVerbatim("FPExceptions") << "\t\td = " << d;
      abort();
    }
  }
} // namespace arttest

DEFINE_ART_MODULE(arttest::FPCTest)
