#include "art/Framework/Core/InputSourceMutex.h"
#include "art/Framework/IO/Root/Inputfwd.h"
// vim: set sw=2 expandtab :

#include "canvas/Utilities/Exception.h"
#include "hep_concurrency/tsan.h"

#include "TBranch.h"
#include "TTree.h"

#include <iostream>

using namespace std;
using namespace hep::concurrency;

namespace art {
  namespace input {

    Int_t
    getEntry(TBranch* branch, EntryNumber entryNumber)
    {
      InputSourceMutexSentry sentry;
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
             unsigned long long& ticks[[gnu::unused]])
    {
      InputSourceMutexSentry sentry;
      try {
        // unsigned tsc_begin_cpuidx = 0;
        // auto tsc_begin = getTSCP(tsc_begin_cpuidx);
        // unsigned tsc_end_cpuidx = tsc_begin_cpuidx;
        // auto tsc_end = tsc_begin;
        auto ret = branch->GetEntry(entryNumber);
        // tsc_end = getTSCP(tsc_end_cpuidx);
        // ticks = tsc_end - tsc_begin;
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
      InputSourceMutexSentry sentry;
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
             unsigned long long& ticks[[gnu::unused]])
    {
      InputSourceMutexSentry sentry;
      try {
        // unsigned tsc_begin_cpuidx = 0;
        // auto tsc_begin = getTSCP(tsc_begin_cpuidx);
        // unsigned tsc_end_cpuidx = tsc_begin_cpuidx;
        // auto tsc_end = tsc_begin;
        auto ret = tree->GetEntry(entryNumber);
        // tsc_end = getTSCP(tsc_end_cpuidx);
        // ticks = tsc_end - tsc_begin;
        // Show the amount of time spent doing the I/O.
        // cerr << "-----> " << __func__ << ": ticks: " << ticks << "\n";
        return ret;
      }
      catch (cet::exception& e) {
        throw art::Exception(art::errors::FileReadError)
          << e.explain_self() << "\n";
      }
    }

  } // namespace input
} // namespace art
