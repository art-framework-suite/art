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
             unsigned long long& ticks)
    {
      RootMutexSentry sentry;
      try {
        unsigned tsc_begin_cpuidx = 0;
        auto tsc_begin = getTSCP(tsc_begin_cpuidx);
        unsigned tsc_end_cpuidx = tsc_begin_cpuidx;
        auto tsc_end = tsc_begin;
        auto ret = branch->GetEntry(entryNumber);
        tsc_end = getTSCP(tsc_end_cpuidx);
        ticks = tsc_end - tsc_begin;
        // Show the amount of time spent doing the I/O.
        // cerr << "-----> " << __func__ << ": ticks: " << ticks << "\n";
        return ret;
      }
      catch (cet::exception& e) {
        throw art::Exception(art::errors::FileReadError)
          << e.explain_self() << "\n";
      }
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
    getEntry(TTree* tree, EntryNumber entryNumber, unsigned long long& ticks)
    {
      RootMutexSentry sentry;
      try {
        unsigned tsc_begin_cpuidx = 0;
        auto tsc_begin = getTSCP(tsc_begin_cpuidx);
        unsigned tsc_end_cpuidx = tsc_begin_cpuidx;
        auto tsc_end = tsc_begin;
        auto ret = tree->GetEntry(entryNumber);
        tsc_end = getTSCP(tsc_end_cpuidx);
        ticks = tsc_end - tsc_begin;
        // Show the amount of time spent doing the I/O.
        // cerr << "-----> " << __func__ << ": ticks: " << ticks << "\n";
        return ret;
      }
      catch (cet::exception& e) {
        throw art::Exception(art::errors::FileReadError)
          << e.explain_self() << "\n";
      }
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
