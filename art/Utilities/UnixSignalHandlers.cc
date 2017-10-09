#include "art/Utilities/UnixSignalHandlers.h"

#include "canvas/Utilities/DebugMacros.h"

#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <string.h>

#if !defined(NSIG)
#if defined(_NSIG)
#define NSIG _NSIG
#elif defined(__DARWIN_NSIG)
#define NSIG __DARWIN_NSIG
#endif
#endif

namespace art {

  std::mutex usr2_lock;

  //--------------------------------------------------------------

  std::atomic<int> shutdown_flag{0};

  extern "C" {
  void
  ep_sigusr2(int which, siginfo_t*, void*)
  {
    FDEBUG(1) << "in sigusr2 handler\n";
    if (which == SIGINT && shutdown_flag > 0) {
      std::cerr << "User signal SIGINT terminating the process\n";
      std::terminate();
    }
    shutdown_flag = which;
  }
  }

  //--------------------------------------------------------------

  std::mutex signum_lock;
  volatile int signum_value =
#if defined(__linux__)
    SIGRTMIN;
#else
    0;
#endif

  //--------------------------------------------------------------

  int
  getSigNum()
  {
    std::lock_guard<decltype(signum_lock)> lock{signum_lock};
    int const rc{signum_value};
    ++signum_value;
    return rc;
  }

#define MUST_BE_ZERO(fun)                                                      \
  if ((fun) != 0) {                                                            \
    perror("UnixSignalHandlers::setupSignal: sig function failed");            \
    abort();                                                                   \
  }

  //--------------------------------------------------------------

  void
  disableAllSigs(sigset_t* oldset)
  {
    sigset_t myset;
    // all blocked for now
    MUST_BE_ZERO(sigfillset(&myset));
    MUST_BE_ZERO(pthread_sigmask(SIG_SETMASK, &myset, oldset));
  }

  //--------------------------------------------------------------

  void
  disableRTSigs()
  {
#if defined(__linux__)
    // ignore all the RT signals
    sigset_t myset;
    MUST_BE_ZERO(sigemptyset(&myset));

    struct sigaction tmpact;
    memset(&tmpact, 0, sizeof(tmpact));
    tmpact.sa_handler = SIG_IGN;

    for (int num = SIGRTMIN; num < SIGRTMAX; ++num) {
      MUST_BE_ZERO(sigaddset(&myset, num));
      MUST_BE_ZERO(sigaction(num, &tmpact, nullptr));
    }

    MUST_BE_ZERO(pthread_sigmask(SIG_BLOCK, &myset, 0));
#endif
  }

  //--------------------------------------------------------------

  void
  reenableSigs(sigset_t* oldset)
  {
    // reenable the signals
    MUST_BE_ZERO(pthread_sigmask(SIG_SETMASK, oldset, 0));
  }

  //--------------------------------------------------------------

  void
  enableSignal(sigset_t* newset, const int signum)
  {
    // enable the specified signal
    MUST_BE_ZERO(sigaddset(newset, signum));
  }

  //--------------------------------------------------------------

  void
  disableSignal(sigset_t* newset, const int signum)
  {
    // disable the specified signal
    MUST_BE_ZERO(sigdelset(newset, signum));
  }

  //--------------------------------------------------------------

  void
  installCustomHandler(const int signum, CFUNC func)
  {
    sigset_t oldset;
    art::disableAllSigs(&oldset);
#if defined(__linux__)
    art::disableRTSigs();
#endif
    art::installSig(signum, func);
    art::reenableSigs(&oldset);
  }

  //--------------------------------------------------------------

  void
  installSig(const int signum, CFUNC func)
  {
    // set up my RT signal now
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = func;
    act.sa_flags = SA_RESTART;

    // get my signal number
    int mysig = signum;
    if (mysig == SIGKILL) {
      perror("Cannot install handler for KILL signal");
      return;
    } else if (mysig == SIGSTOP) {
      perror("Cannot install handler for STOP signal");
      return;
    }

    if (sigaction(mysig, &act, nullptr) != 0) {
      perror("sigaction failed");
      abort();
    }

    sigset_t newset;
    MUST_BE_ZERO(sigemptyset(&newset));
    MUST_BE_ZERO(sigaddset(&newset, mysig));
    MUST_BE_ZERO(pthread_sigmask(SIG_UNBLOCK, &newset, 0));
  }

  //--------------------------------------------------------------

  void
  sigInventory()
  {
    sigset_t tmpset, oldset;
    //    Make a full house set of signals, except for SIGKILL = 9
    //    and SIGSTOP = 19 which cannot be blocked
    MUST_BE_ZERO(sigfillset(&tmpset));
    MUST_BE_ZERO(sigdelset(&tmpset, SIGKILL));
    MUST_BE_ZERO(sigdelset(&tmpset, SIGSTOP));
    //    Swap it with the current sigset_t
    MUST_BE_ZERO(pthread_sigmask(SIG_SETMASK, &tmpset, &oldset));
    //    Now see what's included in the set
    for (int k = 1; k < NSIG; ++k) {
      std::cerr << "sigismember is " << sigismember(&tmpset, k)
                << " for signal " << std::setw(2) << k
#if defined(__linux__)
                << " (" << strsignal(k) << ")"
#endif
                << std::endl;
    }
    //    Finally put the original sigset_t back
    MUST_BE_ZERO(pthread_sigmask(SIG_SETMASK, &oldset, &tmpset));
  }

  void
  setupSignals(bool want_sigint_enabled)
  {
    art::installCustomHandler(SIGUSR2, art::ep_sigusr2);
    art::installCustomHandler(SIGTERM, art::ep_sigusr2);
    art::installCustomHandler(SIGQUIT, art::ep_sigusr2);
    if (want_sigint_enabled)
      art::installCustomHandler(SIGINT, art::ep_sigusr2);
  }

} // art
