#include "test/Framework/Services/Basic/UnitTestClient.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <cmath>
#include <float.h>
#include <iostream>
#include <string>
#include <stdexcept>

#include <csignal>
#include <cstdlib>
#include <memory.h>

extern "C"
{
  typedef void (*SIGFUNC)(int,siginfo_t*,void*);

  void ep_sigfpu(int ,siginfo_t*, void*)
  {
     std::cerr << "Worked!\n";
     exit(0);
  }
}

namespace arttest
{

UnitTestClient::UnitTestClient( fhicl::ParameterSet const & p):
  art::EDAnalyzer(p)
{
  struct sigaction act;
  memset(&act,0,sizeof(act));
  act.sa_sigaction = ep_sigfpu;
  act.sa_flags = SA_RESTART;

  if(sigaction(SIGFPE,&act,0) !=0)
  {
    perror("sigaction failed");
    throw std::runtime_error("cannot install sigaction signal handler");
  }

  sigset_t newset;
  sigemptyset(&newset);
  sigaddset(&newset,SIGFPE);
  pthread_sigmask(SIG_UNBLOCK, &newset,0);
}

UnitTestClient::~UnitTestClient() { }

void UnitTestClient::analyze( art::Event const & e)
{

  double x = 1.0;
  double y = DBL_MAX;

  if(e.id().event() == 2) {
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

    abort();
  }
}

}  // arttest


using arttest::UnitTestClient;
DEFINE_ART_MODULE(UnitTestClient)
