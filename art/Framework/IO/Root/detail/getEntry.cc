#include "art/Framework/IO/Root/Inputfwd.h"
// vim: set sw=2 expandtab :

#include "canvas/Utilities/Exception.h"
#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/tsan.h"

#include "TBranch.h"
#include "TTree.h"

#include <iostream>

using namespace std;
using namespace hep::concurrency;

namespace art {
  namespace input {

    RecursiveMutex* RootMutexSentry::rootMutex_{
      new RecursiveMutex{"art::input::rootMutex_"}};

    Int_t
    getEntry(TBranch* branch, EntryNumber entryNumber)
    {
      RootMutexSentry sentry;
      try {
        return branch->GetEntry(entryNumber);
      }
      catch (cet::exception& e) {
        throw art::Exception(art::errors::FileReadError)
          << e.explain_self() << "\n";
      }
    }

    Int_t
    getEntry(TBranch* branch,
             EntryNumber entryNumber,
             unsigned long long& ticks [[maybe_unused]])
    {
      return getEntry(branch, entryNumber);
    }

    Int_t
    getEntry(TTree* tree, EntryNumber entryNumber)
    {
      RootMutexSentry sentry;
      try {
        return tree->GetEntry(entryNumber);
      }
      catch (cet::exception& e) {
        throw art::Exception(art::errors::FileReadError)
          << e.explain_self() << "\n";
      }
    }

    Int_t
    getEntry(TTree* tree,
             EntryNumber entryNumber,
             unsigned long long& ticks [[maybe_unused]])
    {
      return getEntry(tree, entryNumber);
    }

    RecursiveMutex*
    RootMutexSentry::startup()
    {
      static mutex guard_mutex;
      lock_guard<mutex> sentry(guard_mutex);
      rootMutex_ = new RecursiveMutex{"art::input::rootMutex_"};
      return rootMutex_;
    }

    void
    RootMutexSentry::shutdown()
    {
      ANNOTATE_THREAD_IGNORE_BEGIN;
      delete rootMutex_;
      rootMutex_ = nullptr;
      ANNOTATE_THREAD_IGNORE_END;
    }

    class AutoRootMutexSentryShutdown {
    public:
      ~AutoRootMutexSentryShutdown() { RootMutexSentry::shutdown(); }
      AutoRootMutexSentryShutdown() {}
      AutoRootMutexSentryShutdown(AutoRootMutexSentryShutdown const&) = delete;
      AutoRootMutexSentryShutdown(AutoRootMutexSentryShutdown&&) = delete;
      AutoRootMutexSentryShutdown& operator=(
        AutoRootMutexSentryShutdown const&) = delete;
      AutoRootMutexSentryShutdown& operator=(AutoRootMutexSentryShutdown&&) =
        delete;
    };

    // When this is destroyed at global destruction time,
    // if the the rootMutex_ has not yet been cleaned up
    // it will be done at that time. This is to guard
    // against libraries that do not follow the rules.
    AutoRootMutexSentryShutdown autoRootMutexSentryShutdown;

    RootMutexSentry::~RootMutexSentry() {}

    RootMutexSentry::RootMutexSentry()
      : sentry_{(rootMutex_ == nullptr) ? *startup() : *rootMutex_, __func__}
    {}

  } // namespace input
} // namespace art
