#include "art/Utilities/UnixSignalHandlers.h"
// vim: set sw=2 expandtab :

#include "canvas/Utilities/DebugMacros.h"

#include <atomic>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>

using namespace std;

using my_sigaction_func_t = void (*)(int, siginfo_t*, void*);

atomic<int> art::shutdown_flag{0};

extern "C" {

static void
handle_sigusr2(int which, siginfo_t*, void*)
{
  FDEBUG(1) << "in sigusr2 handler\n";
  if ((which == SIGINT) && (art::shutdown_flag > 0)) {
    cerr << "User signal SIGINT terminating the process\n";
    terminate();
  }
  art::shutdown_flag = which;
}

} // extern "C"

namespace art {

  namespace {

    void
    abort_on_error(int const errcode)
    {
      if (errcode != 0) {
        perror("UnixSignalHandlers::setupSignal: sig function failed");
        abort();
      }
    }

    void
    reenableSigs(sigset_t* oldset)
    {
      abort_on_error(pthread_sigmask(SIG_SETMASK, oldset, 0));
    }

    void
    installSig(const int signum, my_sigaction_func_t func)
    {
      struct sigaction act;
      memset(&act, 0, sizeof(act));
      act.sa_sigaction = func;
      act.sa_flags = SA_RESTART;
      int mysig = signum;
      if (mysig == SIGKILL) {
        cerr << "Cannot install handler for KILL signal\n";
        return;
      } else if (mysig == SIGSTOP) {
        cerr << "Cannot install handler for STOP signal\n";
        return;
      }
      abort_on_error(sigaction(mysig, &act, nullptr));
      sigset_t newset;
      abort_on_error(sigemptyset(&newset));
      abort_on_error(sigaddset(&newset, mysig));
      abort_on_error(pthread_sigmask(SIG_UNBLOCK, &newset, 0));
    }

    void
    disableRTSigs()
    {
#ifdef __linux__
      sigset_t myset;
      abort_on_error(sigemptyset(&myset));
      struct sigaction tmpact;
      memset(&tmpact, 0, sizeof(tmpact));
      tmpact.sa_handler = SIG_IGN;
      for (int num = SIGRTMIN; num < SIGRTMAX; ++num) {
        abort_on_error(sigaddset(&myset, num));
        abort_on_error(sigaction(num, &tmpact, nullptr));
      }
      abort_on_error(pthread_sigmask(SIG_BLOCK, &myset, 0));
#endif // __linux__
    }

    void
    disableAllSigs(sigset_t* oldset)
    {
      sigset_t myset;
      abort_on_error(sigfillset(&myset));
      abort_on_error(pthread_sigmask(SIG_SETMASK, &myset, oldset));
    }

    void
    installCustomHandler(const int signum, my_sigaction_func_t func)
    {
      sigset_t oldset;
      disableAllSigs(&oldset);
#ifdef __linux__
      disableRTSigs();
#endif // __linux__
      installSig(signum, func);
      reenableSigs(&oldset);
    }

  } // unnamed namespace

  void
  setupSignals(bool want_sigint_enabled)
  {
    installCustomHandler(SIGUSR2, handle_sigusr2);
    installCustomHandler(SIGTERM, handle_sigusr2);
    installCustomHandler(SIGQUIT, handle_sigusr2);
    if (want_sigint_enabled) {
      installCustomHandler(SIGINT, handle_sigusr2);
    }
  }

} // namespace art
