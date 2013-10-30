#ifndef art_Utilities_UnixSignalHandlers_h
#define art_Utilities_UnixSignalHandlers_h

/*----------------------------------------------------------------------

UnixSignalHandlers: A set of little utility functions to establish
and manipulate Unix-style signal handling.

----------------------------------------------------------------------*/

#include "art/Utilities/fwd.h"

#include "boost/thread/thread.hpp"

#include <atomic>
#include <csignal>

namespace art {

  extern boost::mutex usr2_lock;
  extern std::atomic<int> shutdown_flag;

  extern "C" {
    void ep_sigusr2(int,siginfo_t*,void*);
    typedef void(*CFUNC)(int,siginfo_t*,void*);
  }

  int getSigNum();
  void disableAllSigs(sigset_t* oldset);
  void disableRTSigs();
  void enableSignal(sigset_t* newset, int signum);
  void disableSignal(sigset_t* newset, int signum);
  void reenableSigs(sigset_t* oldset);
  void installSig(int signum, CFUNC func);
  void installCustomHandler(int signum, CFUNC func);
  void sigInventory();

  void setupSignals(bool want_sigint_enabled);

}  // art

#endif /* art_Utilities_UnixSignalHandlers_h */

// Local Variables:
// mode: c++
// End:
