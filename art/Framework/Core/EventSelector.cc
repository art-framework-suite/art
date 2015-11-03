#include "art/Framework/Core/EventSelector.h"

#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Utilities/RegexMatch.h"
#include "boost/algorithm/string.hpp"
#include "cetlib/container_algorithms.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <cassert>
#include <regex>

using namespace cet;
using namespace std;


namespace art {

  EventSelector::EventSelector(Strings const& pathspecs,
                               Strings const& names)
  {
    init(pathspecs, names);
  }

  EventSelector::EventSelector(Strings const& pathspecs)
    :
    results_from_current_process_(false),
    paths_(pathspecs)
  {}

  EventSelector::EventSelector(fhicl::ParameterSet const& config,
                               Strings const& triggernames)
  {
    auto paths = config.get<Strings>("SelectEvents",{});
    init(paths, triggernames);
  }

  void
  EventSelector::init(Strings const& paths,
                      Strings const& triggernames)
  {
    accept_all_ = false;
    absolute_acceptors_.clear(),
    conditional_acceptors_.clear(),
    exception_acceptors_.clear(),
    all_must_fail_.clear();
    all_must_fail_noex_.clear();
    nTriggerNames_ = triggernames.size();
    notStarPresent_ = false;

    if (paths.empty()) {
      accept_all_ = true;
      return;
    }

    // The following are for the purpose of establishing accept_all_ by
    // virtue of an inclusive set of paths:
    bool unrestricted_star = false;
    bool negated_star      = false;
    bool exception_star    = false;

    for (std::string pathSpecifier : paths) {

      boost::erase_all(pathSpecifier, " \t"); // whitespace eliminated
      if (pathSpecifier == "*")           unrestricted_star = true;
      if (pathSpecifier == "!*")          negated_star = true;
      if (pathSpecifier == "exception@*") exception_star = true;

      string basePathSpec(pathSpecifier);
      bool noex_demanded = false;
      string::size_type
              and_noexception = pathSpecifier.find("&noexception");
      if (and_noexception != string::npos) {
        basePathSpec = pathSpecifier.substr(0,and_noexception);
        noex_demanded = true;
      }
      string::size_type and_noex = pathSpecifier.find("&noex");
      if (and_noex != string::npos) {
        basePathSpec = pathSpecifier.substr(0,and_noexception);
        noex_demanded = true;
      }
      and_noexception = basePathSpec.find("&noexception");
      and_noex = basePathSpec.find("&noex");
      if (and_noexception != string::npos ||
           and_noex != string::npos)
          throw art::Exception(errors::Configuration)
            << "EventSelector::init, An OutputModule is using SelectEvents\n"
               "to request a trigger name, but specifying &noexceptions twice\n"
            << "The improper trigger name is: " << pathSpecifier << "\n";

      string realname(basePathSpec);
      bool negative_criterion = false;
      if (basePathSpec[0] == '!') {
        negative_criterion = true;
        realname = basePathSpec.substr(1,string::npos);
      }
      bool exception_spec = false;
      if (realname.find("exception@") == 0) {
        exception_spec = true;
        realname = realname.substr(10, string::npos);
        // strip off 10 chars, which is length of "exception@"
      }
      if (negative_criterion &&  exception_spec)
          throw art::Exception(errors::Configuration)
            << "EventSelector::init, An OutputModule is using SelectEvents\n"
               "to request a trigger name starting with !exception@.\n"
               "This is not supported.\n"
            << "The improper trigger name is: " << pathSpecifier << "\n";
      if (noex_demanded &&  exception_spec)
          throw art::Exception(errors::Configuration)
            << "EventSelector::init, An OutputModule is using SelectEvents\n"
               "to request a trigger name starting with exception@ "
               "and also demanding no &exceptions.\n"
            << "The improper trigger name is: " << pathSpecifier << "\n";


      // instead of "see if the name can be found in the full list of paths"
      // we want to find all paths that match this name.
      vector<Strings::const_iterator> matches =
              regexMatch(triggernames, realname);

      if (matches.empty() && !is_glob(realname))
      {
          throw art::Exception(errors::Configuration)
            << "EventSelector::init, An OutputModule is using SelectEvents\n"
               "to request a trigger name that does not exist\n"
            << "The unknown trigger name is: " << realname << "\n";
      }
      if (matches.empty() && is_glob(realname))
      {
          mf::LogWarning("Configuration")
            << "EventSelector::init, An OutputModule is using SelectEvents\n"
               "to request a wildcarded trigger name that does not match any trigger \n"
            << "The wildcarded trigger name is: " << realname << "\n";
      }

      auto makeBitInfoPass = [&triggernames](auto m){
        return BitInfo(distance(triggernames.begin(),m), true);
      };

      auto makeBitInfoFail = [&triggernames](auto m){
        return BitInfo(distance(triggernames.begin(),m), false);
      };


      if (!negative_criterion && !noex_demanded && !exception_spec) {
        cet::transform_all( matches,
                            std::back_inserter(absolute_acceptors_),
                            makeBitInfoPass );
      }
      else if (!negative_criterion && noex_demanded) {
        cet::transform_all( matches,
                            std::back_inserter(conditional_acceptors_),
                            makeBitInfoPass );
      }
      else if (exception_spec) {
        cet::transform_all( matches,
                            std::back_inserter(exception_acceptors_),
                            makeBitInfoPass );
      }
      else if (negative_criterion && !noex_demanded) {
        if (matches.empty()) {
          throw art::Exception(errors::Configuration)
            << "EventSelector::init, An OutputModule is using SelectEvents\n"
               "to request all fails on a set of trigger names that do not exist\n"
            << "The problematic name is: " << pathSpecifier << "\n";

        }
        else if (matches.size() == 1) {
          BitInfo bi(distance(triggernames.begin(),matches[0]), false);
          absolute_acceptors_.push_back(bi);
        }
        else {
          Bits mustfail;
          // We set this to false because that will demand bits are Fail.
          cet::transform_all( matches, std::back_inserter(mustfail), makeBitInfoFail );
          all_must_fail_.push_back(mustfail);
        }
      }
      else if (negative_criterion && noex_demanded) {
        if (matches.empty()) {
          throw art::Exception(errors::Configuration)
            << "EventSelector::init, An OutputModule is using SelectEvents\n"
               "to request all fails on a set of trigger names that do not exist\n"
            << "The problematic name is: " << pathSpecifier << "\n";

        }
        else if (matches.size() == 1) {
          BitInfo bi(distance(triggernames.begin(),matches[0]), false);
          conditional_acceptors_.push_back(bi);
        }
        else {
          Bits mustfail;
          cet::transform_all( matches, std::back_inserter(mustfail), makeBitInfoFail );
          all_must_fail_noex_.push_back(mustfail);
        }
      }
    } // for (std::string pathSpecifier : paths)

    if (unrestricted_star && negated_star && exception_star) accept_all_ = true;

  } // EventSelector::init

  bool EventSelector::acceptEvent(TriggerResults const& tr)
  {
    if (accept_all_) return true;

    // For the current process we already initialized in the constructor,
    // The trigger names will not change so we can skip initialization.
    if (!results_from_current_process_) {

      // For previous processes we need to get the trigger names that
      // correspond to the bits in TriggerResults from the ParameterSet
      // set registry, which is stored once per file.  The ParameterSetID
      // stored in TriggerResults is the key used to find the info in the
      // registry.  We optimize using the fact the ID is unique. If the ID
      // has not changed since the last time we initialized with new triggernames,
      // then the names have not changed and we can skip this initialization.
      if (!(psetID_initialized_ && psetID_ == tr.parameterSetID())) {

        Strings triggernames;
        ServiceHandle<TriggerNamesService> tns;
        if (tns->getTrigPaths(tr, triggernames)) {
          init(paths_, triggernames);
          psetID_ = tr.parameterSetID();
          psetID_initialized_ = true;
        }
        // This should never happen
        else {
          throw art::Exception(errors::Unknown)
            << "EventSelector::acceptEvent cannot find the trigger names for\n"
               "a process for which the configuration has requested that the\n"
               "OutputModule use TriggerResults to select events from.  This should\n"
               "be impossible, please send information to reproduce this problem to\n"
               "the ART developers.\n";
        }
      }
    }

    // Now make the decision, based on the supplied TriggerResults tr,
    // which of course can be treated as an HLTGlobalStatus by inheritance

    return selectionDecision(tr);

  } // acceptEvent(TriggerResults const& tr)

  bool
  EventSelector::acceptEvent(unsigned char const* array_of_trigger_results,
                             int number_of_trigger_paths) const
  {

    // This should never occur unless someone uses this function in
    // an incorrect way ...
    if (!results_from_current_process_) {
      throw art::Exception(errors::Configuration)
        << "\nEventSelector.cc::acceptEvent, you are attempting to\n"
        << "use a bit array for trigger results instead of the\n"
        << "TriggerResults object for a previous process.  This\n"
        << "will not work and ought to be impossible\n";
    }

    if (accept_all_) return true;

    // Form HLTGlobalStatus object to represent the array_of_trigger_results
    HLTGlobalStatus tr(number_of_trigger_paths);
    int byteIndex = 0;
    int subIndex  = 0;
    for (int pathIndex = 0; pathIndex < number_of_trigger_paths; ++pathIndex)
    {
      int state = array_of_trigger_results[byteIndex] >> (subIndex * 2);
      state &= 0x3;
      HLTPathStatus pathStatus(static_cast<hlt::HLTState>(state));
      tr[pathIndex] = pathStatus;
      ++subIndex;
      if (subIndex == 4)
      { ++byteIndex;
        subIndex = 0;
      }
    }

    // Now make the decision, based on the HLTGlobalStatus tr,
    // which we have created from the supplied array of results

    return selectionDecision(tr);

  } // acceptEvent(array_of_trigger_results, number_of_trigger_paths)

  bool
  EventSelector::selectionDecision(HLTGlobalStatus const& tr) const
  {
    if (accept_all_) return true;

    bool exceptionPresent = false;
    bool exceptionsLookedFor = false;

    if (acceptOneBit(absolute_acceptors_, tr)) return true;
    if (acceptOneBit(conditional_acceptors_, tr)) {
      exceptionPresent = containsExceptions(tr);
      if (!exceptionPresent) return true;
      exceptionsLookedFor = true;
    }
    if (acceptOneBit(exception_acceptors_, tr, hlt::Exception)) return true;

    for (auto const& f : all_must_fail_) {
      if (acceptAllBits(f, tr)) return true;
    }

    for (auto const& fn : all_must_fail_noex_ ) {
      if (acceptAllBits(fn, tr)) {
        if (!exceptionsLookedFor) {
          exceptionPresent = containsExceptions(tr);
        }
        return !exceptionPresent;
      }
    }

    // If we have not accepted based on any of the acceptors, nor on any one of
    // the all_must_fail_ collections, then we reject this event.

    return false;

  }  // selectionDecision()

// Obsolete...
  bool EventSelector::acceptTriggerPath(HLTPathStatus const& pathStatus,
                                        BitInfo const& pathInfo) const
  {
    return (((pathStatus.state()==hlt::Pass) && (pathInfo.accept_state_)) ||
            ((pathStatus.state()==hlt::Fail) && !(pathInfo.accept_state_)) ||
            ((pathStatus.state()==hlt::Exception)));
  }

  // Indicate if any bit in the trigger results matches the desired value
  // at that position, based on the Bits array.  If s is Exception, this
  // looks for a Exceptionmatch; otherwise, true-->Pass, false-->Fail.
  bool
  EventSelector::acceptOneBit(Bits const& bits,
                              HLTGlobalStatus const& tr,
                              hlt::HLTState const& s) const
  {
    bool lookForException = (s == hlt::Exception);
    for(auto const& b : bits) {
      hlt::HLTState const bstate =
          lookForException ? hlt::Exception
                           : b.accept_state_ ? hlt::Pass
                                             : hlt::Fail;
      if (tr[b.pos_].state() == bstate) return true;
    }
    return false;
  } // acceptOneBit

  // Indicate if *every* bit in the trigger results matches the desired value
  // at that position, based on the Bits array: true-->Pass, false-->Fail.
  bool
  EventSelector::acceptAllBits(Bits const& bits,
                               HLTGlobalStatus const& tr) const
  {
    for(auto const& b : bits) {
      hlt::HLTState const bstate = b.accept_state_ ? hlt::Pass : hlt::Fail;
      if (tr[b.pos_].state() != bstate) return false;
    }
    return true;
  } // acceptAllBits

  /**
   * Applies a trigger selection mask to a specified trigger result object.
   * Within the trigger result object, each path status is left unchanged
   * if it satisfies the trigger selection (path specs) or cleared if it
   * does not satisfy the trigger selection.  In this way, the resulting
   * trigger result object contains only path status values that "pass"
   * the selection criteria.
   *
   * @param inputResults The raw trigger results object that will be masked.
   * @return a copy of the input trigger results object with only the path
   *         status results that match the trigger selection.
   * @throws art::Exception if the number of paths in the TriggerResults
   *         object does not match the specified full trigger list, or
   *         if the trigger selection is invalid in the context of the
   *         full trigger list.
   */
  std::shared_ptr<TriggerResults>
  EventSelector::maskTriggerResults(TriggerResults const& inputResults)
  {
    // fetch and validate the total number of paths
    unsigned int fullTriggerCount = nTriggerNames_;
    unsigned int N = fullTriggerCount;
    if (fullTriggerCount != inputResults.size())
    {
      throw art::Exception(errors::DataCorruption)
        << "EventSelector::maskTriggerResults, the TriggerResults\n"
        << "size (" << inputResults.size()
        << ") does not match the number of paths in the\n"
        << "full trigger list (" << fullTriggerCount << ").\n";
    }

    // create a suitable global status object to work with, all in Ready state
    HLTGlobalStatus mask(fullTriggerCount);

    // Deal with must_fail acceptors that would cause selection
    for (unsigned int m = 0; m < this->all_must_fail_.size(); ++m) {
      vector<bool>
        f = expandDecisionList(this->all_must_fail_[m],false,N);
      bool all_fail = true;
      for (unsigned int ipath = 0; ipath < N; ++ipath) {
        if  ((f[ipath]) && (inputResults [ipath].state() != hlt::Fail)) {
          all_fail = false;
          break;
        }
      }
      if (all_fail) {
        for (unsigned int ipath = 0; ipath < N; ++ipath) {
          if  (f[ipath]) {
            mask[ipath] = hlt::Fail;
          }
        }
      }
    }
    for (unsigned int m = 0; m < this->all_must_fail_noex_.size(); ++m) {
      vector<bool>
        f = expandDecisionList(this->all_must_fail_noex_[m],false,N);
      bool all_fail = true;
      for (unsigned int ipath = 0; ipath < N; ++ipath) {
        if ((f[ipath]) && (inputResults [ipath].state() != hlt::Fail)) {
          all_fail = false;
          break;
        }
      }
      if (all_fail) {
        for (unsigned int ipath = 0; ipath < N; ++ipath) {
          if  (f[ipath]) {
            mask[ipath] = hlt::Fail;
          }
        }
      }
    } // factoring opportunity - work done for fail_noex_ is same as for fail_

    // Deal with normal acceptors that would cause selection
    vector<bool> aPassAbs = expandDecisionList(this->absolute_acceptors_   ,true ,N);
    vector<bool> aPassCon = expandDecisionList(this->conditional_acceptors_,true ,N);
    vector<bool> aFailAbs = expandDecisionList(this->absolute_acceptors_   ,false,N);
    vector<bool> aFailCon = expandDecisionList(this->conditional_acceptors_,false,N);
    vector<bool> aExc     = expandDecisionList(this->exception_acceptors_  ,true ,N);

    for (unsigned int ipath = 0; ipath < N; ++ipath) {
      hlt::HLTState s = inputResults [ipath].state();
      if (((aPassAbs[ipath]) && (s == hlt::Pass))
                ||
          ((aPassCon[ipath]) && (s == hlt::Pass))
                ||
          ((aFailAbs[ipath]) && (s == hlt::Fail))
                ||
          ((aFailCon[ipath]) && (s == hlt::Fail))
                ||
          ((aExc[ipath]) && (s == hlt::Exception)))
      {
        mask[ipath] = s;
      }
    }

    // Based on the global status for the mask, create and return a
    // TriggerResults
    return std::make_shared<TriggerResults>(mask, inputResults.parameterSetID());
  }  // maskTriggerResults

  bool EventSelector::containsExceptions(HLTGlobalStatus const& tr) const
  {
    unsigned int e = tr.size();
    for (unsigned int i = 0; i < e; ++i) {
      if (tr[i].state() == hlt::Exception) return true;
    }
    return false;
  }

  // The following routines are helpers for testSelectionOverlap

  bool
  EventSelector::identical(vector<bool> const& a,
                           vector<bool> const& b) {
     unsigned int n = a.size();
     if (n != b.size()) return false;
     for (unsigned int i=0; i!=n; ++i) {
       if (a[i] != b[i]) return false;
     }
     return true;
  }

  bool
  EventSelector::identical(EventSelector const& a,
                           EventSelector const& b,
                           unsigned int N)
  {
        // create the expanded masks for the various decision lists in a and b
    if (!identical(expandDecisionList(a.absolute_acceptors_,true,N),
                   expandDecisionList(b.absolute_acceptors_,true,N)))
                   return false;
    if (!identical(expandDecisionList(a.conditional_acceptors_,true,N),
                   expandDecisionList(b.conditional_acceptors_,true,N)))
                   return false;
    if (!identical(expandDecisionList(a.absolute_acceptors_,false,N),
                   expandDecisionList(b.absolute_acceptors_,false,N)))
                   return false;
    if (!identical(expandDecisionList(a.conditional_acceptors_,false,N),
                   expandDecisionList(b.conditional_acceptors_,false,N)))
                   return false;
    if (!identical(expandDecisionList(a.exception_acceptors_,true,N),
                   expandDecisionList(b.exception_acceptors_,true,N)))
                   return false;
    if (a.all_must_fail_.size() != b.all_must_fail_.size()) return false;

    vector< vector<bool> > aMustFail;
    for (unsigned int m = 0; m != a.all_must_fail_.size(); ++m) {
      aMustFail.push_back(expandDecisionList(a.all_must_fail_[m],false,N));
    }
    vector< vector<bool> > aMustFailNoex;
    for (unsigned int m = 0; m != a.all_must_fail_noex_.size(); ++m) {
      aMustFailNoex.push_back
              (expandDecisionList(a.all_must_fail_noex_[m],false,N));
    }
    vector< vector<bool> > bMustFail;
    for (unsigned int m = 0; m != b.all_must_fail_.size(); ++m) {
      bMustFail.push_back(expandDecisionList(b.all_must_fail_[m],false,N));
    }
    vector< vector<bool> > bMustFailNoex;
    for (unsigned int m = 0; m != b.all_must_fail_noex_.size(); ++m) {
      bMustFailNoex.push_back
              (expandDecisionList(b.all_must_fail_noex_[m],false,N));
    }

    for (unsigned int m = 0; m != aMustFail.size(); ++m) {
      bool match = false;
      for (unsigned int k = 0; k != bMustFail.size(); ++k) {
        if (identical(aMustFail[m],bMustFail[k])) {
          match = true;
          break;
        }
      }
      if (!match) return false;
    }
    for (unsigned int m = 0; m != aMustFailNoex.size(); ++m) {
      bool match = false;
      for (unsigned int k = 0; k != bMustFailNoex.size(); ++k) {
         if (identical(aMustFailNoex[m],bMustFailNoex[k])) {
          match = true;
          break;
        }
      }
      if (!match) return false;
    }

    return true;

  } // identical (EventSelector, EventSelector, N);

  vector<bool>
  EventSelector::expandDecisionList(Bits const& b,
                                      bool PassOrFail,
                                      unsigned int n)
  {
    vector<bool> x(n, false);
    for (unsigned int i = 0; i != b.size(); ++i) {
      if (b[i].accept_state_ == PassOrFail) x[b[i].pos_] = true;
    }
    return x;
  } // expandDecisionList

  // Determines whether a and b share a true bit at any position
  bool EventSelector::overlapping(vector<bool> const& a,
                                     vector<bool> const& b)
  {
    if (a.size() != b.size()) return false;
    for (unsigned int i = 0; i != a.size(); ++i) {
      if (a[i] && b[i]) return true;
    }
    return false;
  } // overlapping

  // determines whether the true bits of a are a non-empty subset of those of b,
  // or vice-versa.  The subset need not be proper.
  bool EventSelector::subset(vector<bool> const& a,
                             vector<bool> const& b)
  {
    if (a.size() != b.size()) return false;
    // First test whether a is a non-empty subset of b
    bool aPresent = false;
    bool aSubset = true;
    for (unsigned int i = 0; i != a.size(); ++i) {
      if (a[i]) {
        aPresent = true;
        if (!b[i]) {
          aSubset = false;
          break;
        }
      }
    }
    if (!aPresent) return false;
    if (aSubset) return true;

    // Now test whether b is a non-empty subset of a
    bool bPresent = false;
    bool bSubset = true;
    for (unsigned int i = 0; i != b.size(); ++i) {
      if (b[i]) {
        bPresent = true;
        if (!a[i]) {
          bSubset = false;
          break;
        }
      }
    }
    if (!bPresent) return false;
    if (bSubset) return true;

    return false;
  } // subset

  // Creates a vector of bits which is the OR of a and b
  vector<bool>
  EventSelector::combine(vector<bool> const& a,
                         vector<bool> const& b)
  {
    assert(a.size() == b.size());
    vector<bool> x(a.size());
    for (unsigned int i = 0; i != a.size(); ++i) {
      x[i] = a[i] || b[i];
    } // a really sharp compiler will optimize the hell out of this,
      // exploiting word-size OR operations.
    return x;
  } // combine

}  // art
