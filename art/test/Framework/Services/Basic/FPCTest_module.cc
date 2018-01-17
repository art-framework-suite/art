#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "fpc_utils.h"

#include <cmath>
#include <csignal>
#include <cstdlib>
#include <float.h>
#include <iostream>
#include <memory.h>
#include <stdexcept>
#include <string>

extern "C" {
void
ep_sigfpu(int, siginfo_t*, void*)
{
  std::cerr << "Worked!\n";
  exit(0);
}
}

namespace arttest {

  class FPCTest : public art::EDAnalyzer {
  public:
    explicit FPCTest(fhicl::ParameterSet const& p) : art::EDAnalyzer{p}
    {
      struct sigaction act;
      memset(&act, 0, sizeof(act));
      act.sa_sigaction = ep_sigfpu;
      act.sa_flags = SA_RESTART;

      if (sigaction(SIGFPE, &act, 0) != 0) {
        perror("sigaction failed");
        throw std::runtime_error("cannot install sigaction signal handler");
      }

      sigset_t newset;
      sigemptyset(&newset);
      sigaddset(&newset, SIGFPE);
      pthread_sigmask(SIG_UNBLOCK, &newset, 0);
    }

  private:
    void
    analyze(art::Event const& e) override
    {
      double const x{1.0};
      double const y{DBL_MAX};

      if (e.id().event() == 2) {
        mf::LogVerbatim("FPExceptions") << "\n\t\tx = " << x;
        mf::LogVerbatim("FPExceptions") << "\t\ty = " << y << " (DBL_MAX)";

        // DivideByZero
        mf::LogVerbatim("FPExceptions") << "\t\tForce DivideByZero: a = x/zero";
        double const a{divit(x, 0)};
        mf::LogVerbatim("FPExceptions") << "\t\ta = " << a;

        // Invalid
        mf::LogVerbatim("FPExceptions")
          << "\t\tForce Invalid: b = std::log(-1.0)";
        double const b{std::log(-1.0)};
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
  };

} // arttest

DEFINE_ART_MODULE(arttest::FPCTest)
