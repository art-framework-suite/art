#include "art/Framework/Core/EventSelector.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Utilities/RegexMatch.h"
#include "boost/algorithm/string.hpp"
#include "canvas/Persistency/Common/HLTPathStatus.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <regex>
#include <string>
#include <vector>

using namespace cet;
using namespace std;

namespace art {

  EventSelector::EventSelector(vector<string> const& pathspecs,
                               vector<string> const& trigger_path_names)
    : accept_all_{false}
    , absolute_acceptors_{}
    , conditional_acceptors_{}
    , exception_acceptors_{}
    , all_must_fail_{}
    , all_must_fail_noex_{}
    , force_results_from_current_process_{true}
    , psetID_initialized_{false}
    , psetID_{}
    , pathspecs_{}
    , nTriggerNames_{0}
    , notStarPresent_{false}
  {
    init(pathspecs, trigger_path_names);
  }

  EventSelector::EventSelector(vector<string> const& pathspecs)
    : accept_all_{false}
    , absolute_acceptors_{}
    , conditional_acceptors_{}
    , exception_acceptors_{}
    , all_must_fail_{}
    , all_must_fail_noex_{}
    , force_results_from_current_process_{false}
    , psetID_initialized_{false}
    , psetID_{}
    , pathspecs_(pathspecs)
    , nTriggerNames_{0}
    , notStarPresent_{false}
  {}

  bool
  EventSelector::wantAll() const
  {
    return accept_all_;
  }

  void
  EventSelector::init(vector<string> const& pathspecs,
                      vector<string> const& trigger_path_names)
  {
    accept_all_ = false;
    absolute_acceptors_.clear();
    conditional_acceptors_.clear();
    exception_acceptors_.clear();
    all_must_fail_.clear();
    all_must_fail_noex_.clear();
    nTriggerNames_ = trigger_path_names.size();
    notStarPresent_ = false;
    if (pathspecs.empty()) {
      accept_all_ = true;
      return;
    }
    // The following are for the purpose of establishing accept_all_ by
    // virtue of an inclusive set of paths.
    bool unrestricted_star = false;
    bool negated_star = false;
    bool exception_star = false;
    for (string const& pathSpecifier : pathspecs) {
      string specifier{pathSpecifier};
      // whitespace eliminated
      boost::erase_all(specifier, " \t");
      if (specifier == "*") {
        unrestricted_star = true;
      }
      if (specifier == "!*") {
        negated_star = true;
      }
      if (specifier == "exception@*") {
        exception_star = true;
      }
      // Remove "&noexception" from specifier
      bool noex_demanded = false;
      string const& noexLiteral{"&noexception"};
      auto const noexception_pos = specifier.find(noexLiteral);
      if (noexception_pos != string::npos) {
        if ((noexception_pos + noexLiteral.length()) < specifier.length()) {
          throw Exception(errors::Configuration)
            << "EventSelector::init, An OutputModule is using SelectEvents\n"
            << "to request a trigger name that has extra characters after "
               "'&noexception'.\n"
            << "The improper trigger name is: " << pathSpecifier << "\n";
        }
        specifier.erase(noexception_pos);
        noex_demanded = true;
      }
      // Remove '!' and "exception@"
      bool negative_criterion = false;
      if (specifier[0] == '!') {
        negative_criterion = true;
        specifier.erase(0, 1);
      }
      bool exception_spec = false;
      string const& exLiteral{"exception@"};
      auto const pos = specifier.find(exLiteral);
      if (pos == 0) {
        exception_spec = true;
        specifier.erase(0, exLiteral.length());
      } else if (pos != string::npos) {
        throw Exception(errors::Configuration)
          << "EventSelector::init, An OutputModule is using SelectEvents\n"
          << "to request a trigger name that has disallowed characters before "
             "'exception@'.\n"
          << "The improper trigger name is: " << pathSpecifier << "\n";
      }
      if (negative_criterion && exception_spec)
        throw Exception(errors::Configuration)
          << "EventSelector::init, An OutputModule is using SelectEvents\n"
          << "to request a trigger name starting with !exception@.\n"
          << "This is not supported.\n"
          << "The improper trigger name is: " << pathSpecifier << "\n";
      if (noex_demanded && exception_spec)
        throw Exception(errors::Configuration)
          << "EventSelector::init, An OutputModule is using SelectEvents\n"
          << "to request a trigger name starting with exception@ "
          << "and also demanding &noexception.\n"
          << "The improper trigger name is: " << pathSpecifier << "\n";
      // instead of "see if the name can be found in the full list of
      // paths" we want to find all paths that match this name.
      //
      // 'specifier' now corresponds to the real trigger-path name,
      // free of any decorations
      string const& realname{specifier};
      vector<vector<string>::const_iterator> matches =
        regexMatch(trigger_path_names, realname);
      if (matches.empty()) {
        if (is_glob(realname)) {
          mf::LogWarning("Configuration")
            << "EventSelector::init, An OutputModule is "
               "using SelectEvents\n"
               "to request a wildcarded trigger name that "
               "does not match any trigger \n"
               "The wildcarded trigger name is: "
            << realname
            << " (from trigger-path specification: " << pathSpecifier << ") \n";
        } else {
          throw Exception(errors::Configuration)
            << "EventSelector::init, An OutputModule is "
               "using SelectEvents\n"
               "to request a trigger name that does not "
               "exist\n"
               "The unknown trigger name is: "
            << realname
            << " (from trigger-path specification: " << pathSpecifier << ") \n";
        }
      }
      auto makeBitInfoPass = [&trigger_path_names](auto m) {
        return BitInfo(distance(trigger_path_names.begin(), m), true);
      };
      auto makeBitInfoFail = [&trigger_path_names](auto m) {
        return BitInfo(distance(trigger_path_names.begin(), m), false);
      };
      if (!negative_criterion && !noex_demanded && !exception_spec) {
        cet::transform_all(
          matches, back_inserter(absolute_acceptors_), makeBitInfoPass);
      } else if (!negative_criterion && noex_demanded) {
        cet::transform_all(
          matches, back_inserter(conditional_acceptors_), makeBitInfoPass);
      } else if (exception_spec) {
        cet::transform_all(
          matches, back_inserter(exception_acceptors_), makeBitInfoPass);
      } else if (negative_criterion && !noex_demanded) {
        if (matches.empty()) {
          throw Exception(errors::Configuration)
            << "EventSelector::init, An OutputModule is using SelectEvents\n"
               "to request all fails on a set of trigger names that do not "
               "exist\n"
            << "The problematic name is: " << pathSpecifier << "\n";
        } else if (matches.size() == 1) {
          BitInfo bi(distance(trigger_path_names.begin(), matches[0]), false);
          absolute_acceptors_.push_back(bi);
        } else {
          vector<BitInfo> mustfail;
          // We set this to false because that will demand bits are Fail.
          cet::transform_all(matches, back_inserter(mustfail), makeBitInfoFail);
          all_must_fail_.push_back(mustfail);
        }
      } else if (negative_criterion && noex_demanded) {
        if (matches.empty()) {
          throw Exception(errors::Configuration)
            << "EventSelector::init, An OutputModule is using SelectEvents\n"
               "to request all fails on a set of trigger names that do not "
               "exist\n"
            << "The problematic name is: " << pathSpecifier << "\n";
        } else if (matches.size() == 1) {
          BitInfo bi(distance(trigger_path_names.begin(), matches[0]), false);
          conditional_acceptors_.push_back(bi);
        } else {
          vector<BitInfo> mustfail;
          cet::transform_all(matches, back_inserter(mustfail), makeBitInfoFail);
          all_must_fail_noex_.push_back(mustfail);
        }
      }
    }
    if (unrestricted_star && negated_star && exception_star) {
      accept_all_ = true;
    }
  }

  bool
  EventSelector::acceptEvent(TriggerResults const& tr)
  {
    if (accept_all_) {
      return true;
    }
    if (force_results_from_current_process_) {
      return selectionDecision(tr);
    }
    if (psetID_initialized_ && (psetID_ == tr.parameterSetID())) {
      return selectionDecision(tr);
    }
    // Reinitialize with trigger names from tr.
    vector<string> trigger_path_names;
    fhicl::ParameterSet pset;
    if (!fhicl::ParameterSetRegistry::get(tr.parameterSetID(), pset)) {
      // This should never happen
      throw Exception(errors::Unknown)
        << "EventSelector::acceptEvent cannot find the trigger names for\n"
        << "a process for which the configuration has requested that the\n"
        << "OutputModule use TriggerResults to select events from.  This "
           "should\n"
        << "be impossible, please send information to reproduce this problem "
           "to\n"
        << "the art developers at artists@fnal.gov.\n";
    }
    trigger_path_names = pset.get<vector<string>>("trigger_paths", {});
    if (trigger_path_names.size() != tr.size()) {
      throw Exception(errors::Unknown)
        << "EventSelector::acceptEvent: Trigger names vector and\n"
        << "TriggerResults are different sizes.  This should be impossible,\n"
        << "please send information to reproduce this problem to\n"
        << "the ART developers.\n";
    }
    init(pathspecs_, trigger_path_names);
    psetID_ = tr.parameterSetID();
    psetID_initialized_ = true;
    return selectionDecision(tr);
  }

  bool
  EventSelector::selectionDecision(HLTGlobalStatus const& tr) const
  {
    if (accept_all_) {
      return true;
    }
    bool exceptionPresent = false;
    bool exceptionsLookedFor = false;
    if (acceptOneBit(absolute_acceptors_, tr)) {
      return true;
    }
    if (acceptOneBit(conditional_acceptors_, tr)) {
      exceptionPresent = containsExceptions(tr);
      if (!exceptionPresent) {
        return true;
      }
      exceptionsLookedFor = true;
    }
    if (acceptOneBit(exception_acceptors_, tr, hlt::Exception)) {
      return true;
    }
    for (auto const& f : all_must_fail_) {
      if (acceptAllBits(f, tr)) {
        return true;
      }
    }
    for (auto const& fn : all_must_fail_noex_) {
      if (acceptAllBits(fn, tr)) {
        if (!exceptionsLookedFor) {
          exceptionPresent = containsExceptions(tr);
        }
        return !exceptionPresent;
      }
    }
    return false;
  }

  // Indicate if any bit in the trigger results matches the desired value
  // at that position, based on the bits array.  If s is Exception, this
  // looks for a Exceptionmatch; otherwise, true-->Pass, false-->Fail.
  bool
  EventSelector::acceptOneBit(vector<BitInfo> const& bits,
                              HLTGlobalStatus const& tr,
                              hlt::HLTState const& s) const
  {
    bool lookForException = (s == hlt::Exception);
    for (auto const& b : bits) {
      hlt::HLTState const bstate = lookForException ?
                                     hlt::Exception :
                                     b.accept_state_ ? hlt::Pass : hlt::Fail;
      if (tr[b.pos_].state() == bstate) {
        return true;
      }
    }
    return false;
  }

  // Indicate if *every* bit in the trigger results matches the desired value
  // at that position, based on the bits array: true-->Pass, false-->Fail.
  bool
  EventSelector::acceptAllBits(vector<BitInfo> const& bits,
                               HLTGlobalStatus const& tr) const
  {
    for (auto const& b : bits) {
      hlt::HLTState const bstate = b.accept_state_ ? hlt::Pass : hlt::Fail;
      if (tr[b.pos_].state() != bstate) {
        return false;
      }
    }
    return true;
  }

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
   * @throws Exception if the number of paths in the TriggerResults
   *         object does not match the specified full trigger list, or
   *         if the trigger selection is invalid in the context of the
   *         full trigger list.
   */
  shared_ptr<TriggerResults>
  EventSelector::maskTriggerResults(TriggerResults const& inputResults)
  {
    // fetch and validate the total number of paths
    unsigned int const N = nTriggerNames_;
    if (N != inputResults.size()) {
      throw Exception(errors::DataCorruption)
        << "EventSelector::maskTriggerResults, the TriggerResults\n"
        << "size (" << inputResults.size()
        << ") does not match the number of paths in the\n"
        << "full trigger list (" << N << ").\n";
    }
    // create a suitable global status object to work with, all in Ready state
    HLTGlobalStatus mask{N};
    // Deal with must_fail acceptors that would cause selection
    for (auto const& mf : all_must_fail_) {
      vector<bool> const f = expandDecisionList(mf, false, N);
      bool all_fail = true;
      for (unsigned int ipath = 0; ipath < N; ++ipath) {
        if ((f[ipath]) && (inputResults[ipath].state() != hlt::Fail)) {
          all_fail = false;
          break;
        }
      }
      if (all_fail) {
        for (unsigned int ipath = 0; ipath < N; ++ipath) {
          if (f[ipath]) {
            mask[ipath] = HLTPathStatus(hlt::Fail);
          }
        }
      }
    }
    for (auto const& mf_noex : all_must_fail_noex_) {
      vector<bool> const f = expandDecisionList(mf_noex, false, N);
      bool all_fail = true;
      for (unsigned int ipath = 0; ipath < N; ++ipath) {
        if ((f[ipath]) && (inputResults[ipath].state() != hlt::Fail)) {
          all_fail = false;
          break;
        }
      }
      if (all_fail) {
        for (unsigned int ipath = 0; ipath < N; ++ipath) {
          if (f[ipath]) {
            mask[ipath] = HLTPathStatus(hlt::Fail);
          }
        }
      }
    }
    // Deal with normal acceptors that would cause selection
    vector<bool> aPassAbs = expandDecisionList(absolute_acceptors_, true, N);
    vector<bool> aPassCon = expandDecisionList(conditional_acceptors_, true, N);
    vector<bool> aFailAbs = expandDecisionList(absolute_acceptors_, false, N);
    vector<bool> aFailCon =
      expandDecisionList(conditional_acceptors_, false, N);
    vector<bool> aExc = expandDecisionList(exception_acceptors_, true, N);
    for (unsigned int ipath = 0; ipath < N; ++ipath) {
      hlt::HLTState s = inputResults[ipath].state();
      if (((aPassAbs[ipath]) && (s == hlt::Pass)) ||
          ((aPassCon[ipath]) && (s == hlt::Pass)) ||
          ((aFailAbs[ipath]) && (s == hlt::Fail)) ||
          ((aFailCon[ipath]) && (s == hlt::Fail)) ||
          ((aExc[ipath]) && (s == hlt::Exception))) {
        mask[ipath] = HLTPathStatus(s);
      }
    }
    // Based on the global status for the mask, create and return a
    // TriggerResults
    return make_shared<TriggerResults>(mask, inputResults.parameterSetID());
  }

  bool
  EventSelector::containsExceptions(HLTGlobalStatus const& tr) const
  {
    unsigned int e = tr.size();
    for (unsigned int i = 0; i < e; ++i) {
      if (tr[i].state() == hlt::Exception) {
        return true;
      }
    }
    return false;
  }

  bool
  EventSelector::identical(vector<bool> const& a, vector<bool> const& b)
  {
    unsigned int n = a.size();
    if (n != b.size()) {
      return false;
    }
    for (unsigned int i = 0; i != n; ++i) {
      if (a[i] != b[i]) {
        return false;
      }
    }
    return true;
  }

  bool
  EventSelector::identical(EventSelector const& a,
                           EventSelector const& b,
                           unsigned int N)
  {
    // create the expanded masks for the various decision lists in a and b
    if (!identical(expandDecisionList(a.absolute_acceptors_, true, N),
                   expandDecisionList(b.absolute_acceptors_, true, N))) {
      return false;
    }
    if (!identical(expandDecisionList(a.conditional_acceptors_, true, N),
                   expandDecisionList(b.conditional_acceptors_, true, N))) {
      return false;
    }
    if (!identical(expandDecisionList(a.absolute_acceptors_, false, N),
                   expandDecisionList(b.absolute_acceptors_, false, N))) {
      return false;
    }
    if (!identical(expandDecisionList(a.conditional_acceptors_, false, N),
                   expandDecisionList(b.conditional_acceptors_, false, N))) {
      return false;
    }
    if (!identical(expandDecisionList(a.exception_acceptors_, true, N),
                   expandDecisionList(b.exception_acceptors_, true, N))) {
      return false;
    }
    if (a.all_must_fail_.size() != b.all_must_fail_.size()) {
      return false;
    }
    vector<vector<bool>> aMustFail;
    for (unsigned int m = 0; m != a.all_must_fail_.size(); ++m) {
      aMustFail.push_back(expandDecisionList(a.all_must_fail_[m], false, N));
    }
    vector<vector<bool>> aMustFailNoex;
    for (unsigned int m = 0; m != a.all_must_fail_noex_.size(); ++m) {
      aMustFailNoex.push_back(
        expandDecisionList(a.all_must_fail_noex_[m], false, N));
    }
    vector<vector<bool>> bMustFail;
    for (unsigned int m = 0; m != b.all_must_fail_.size(); ++m) {
      bMustFail.push_back(expandDecisionList(b.all_must_fail_[m], false, N));
    }
    vector<vector<bool>> bMustFailNoex;
    for (unsigned int m = 0; m != b.all_must_fail_noex_.size(); ++m) {
      bMustFailNoex.push_back(
        expandDecisionList(b.all_must_fail_noex_[m], false, N));
    }
    for (unsigned int m = 0; m != aMustFail.size(); ++m) {
      bool match = false;
      for (unsigned int k = 0; k != bMustFail.size(); ++k) {
        if (identical(aMustFail[m], bMustFail[k])) {
          match = true;
          break;
        }
      }
      if (!match) {
        return false;
      }
    }
    for (unsigned int m = 0; m != aMustFailNoex.size(); ++m) {
      bool match = false;
      for (unsigned int k = 0; k != bMustFailNoex.size(); ++k) {
        if (identical(aMustFailNoex[m], bMustFailNoex[k])) {
          match = true;
          break;
        }
      }
      if (!match) {
        return false;
      }
    }
    return true;
  }

  vector<bool>
  EventSelector::expandDecisionList(vector<BitInfo> const& b,
                                    bool PassOrFail,
                                    unsigned int n)
  {
    vector<bool> x(n, false);
    for (unsigned int i = 0; i != b.size(); ++i) {
      if (b[i].accept_state_ == PassOrFail) {
        x[b[i].pos_] = true;
      }
    }
    return x;
  }

  // Determines whether a and b share a true bit at any position
  bool
  EventSelector::overlapping(vector<bool> const& a, vector<bool> const& b)
  {
    if (a.size() != b.size()) {
      return false;
    }
    for (unsigned int i = 0; i != a.size(); ++i) {
      if (a[i] && b[i]) {
        return true;
      }
    }
    return false;
  }

  // determines whether the true bits of a are a non-empty subset of those of b,
  // or vice-versa.  The subset need not be proper.
  bool
  EventSelector::subset(vector<bool> const& a, vector<bool> const& b)
  {
    if (a.size() != b.size()) {
      return false;
    }
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
    if (!aPresent) {
      return false;
    }
    if (aSubset) {
      return true;
    }
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
    if (!bPresent) {
      return false;
    }
    if (bSubset) {
      return true;
    }
    return false;
  }

  // Creates a vector of bits which is the OR of a and b
  vector<bool>
  EventSelector::combine(vector<bool> const& a, vector<bool> const& b)
  {
    assert(a.size() == b.size());
    vector<bool> x(a.size());
    for (unsigned int i = 0; i != a.size(); ++i) {
      x[i] = a[i] || b[i];
    }
    return x;
  }

} // namespace art
