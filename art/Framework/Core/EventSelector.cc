#include "art/Framework/Core/EventSelector.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/detail/RegexMatch.h"
#include "art/Utilities/detail/remove_whitespace.h"
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

namespace {
  unsigned int
  path_position(vector<string> const& trigger_path_names,
                vector<string>::const_iterator it)
  {
    return distance(trigger_path_names.begin(), it);
  }

  bool
  remove_noexception(std::string const& full_path_specifier,
                     std::string& specifier)
  {
    string const noexLiteral{"&noexception"};
    auto const noexception_pos = specifier.find(noexLiteral);
    if (noexception_pos == string::npos) {
      return false;
    }

    if ((noexception_pos + noexLiteral.length()) < specifier.length()) {
      throw art::Exception(art::errors::Configuration)
        << "EventSelector::init, A module is using SelectEvents\n"
        << "to request a trigger name that has extra characters after "
           "'&noexception'.\n"
        << "The improper trigger name is: " << full_path_specifier << "\n";
    }
    specifier.erase(noexception_pos);
    return true;
  }

  bool
  remove_negation(std::string& specifier)
  {
    if (specifier[0] != '!') {
      return false;
    }
    specifier.erase(0, 1);
    return true;
  }

  bool
  remove_exception(std::string const& full_path_specifier,
                   std::string& specifier)
  {
    // Remove "exception@"
    string const exLiteral{"exception@"};
    auto const pos = specifier.find(exLiteral);
    if (pos == 0) {
      specifier.erase(0, exLiteral.length());
      return true;
    }

    // Any other non-npos position is illegal.
    if (pos != string::npos) {
      throw art::Exception(art::errors::Configuration)
        << "EventSelector::init, A module is using SelectEvents\n"
        << "to request a trigger name that has disallowed characters before "
           "'exception@'.\n"
        << "The improper trigger name is: " << full_path_specifier << "\n";
    }
    return false;
  }

}

namespace art {

  EventSelector::EventSelector(vector<string> const& pathspecs,
                               vector<string> const& trigger_path_names)
  {
    init(pathspecs, trigger_path_names);
  }

  EventSelector::EventSelector(vector<string> const& pathspecs)
    : force_results_from_current_process_{false}, pathspecs_{pathspecs}
  {}

  void
  EventSelector::init(vector<string> const& pathspecs,
                      vector<string> const& trigger_path_specs)
  {
    accept_all_ = false;
    absolute_acceptors_.clear();
    conditional_acceptors_.clear();
    exception_acceptors_.clear();
    all_must_fail_.clear();
    all_must_fail_noex_.clear();

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

      assert(not detail::has_whitespace(specifier));

      if (specifier == "*") {
        unrestricted_star = true;
      }
      if (specifier == "!*") {
        negated_star = true;
      }
      if (specifier == "exception@*") {
        exception_star = true;
      }

      bool const noex_demanded = remove_noexception(pathSpecifier, specifier);
      bool const negative_criterion = remove_negation(specifier);
      bool const exception_spec = remove_exception(pathSpecifier, specifier);

      if (negative_criterion && exception_spec) {
        throw Exception(errors::Configuration)
          << "EventSelector::init, A module is using SelectEvents\n"
          << "to request a trigger name starting with !exception@.\n"
          << "This is not supported.\n"
          << "The improper trigger name is: " << pathSpecifier << "\n";
      }

      if (noex_demanded && exception_spec) {
        throw Exception(errors::Configuration)
          << "EventSelector::init, A module is using SelectEvents\n"
          << "to request a trigger name starting with exception@ "
          << "and also demanding &noexception.\n"
          << "The improper trigger name is: " << pathSpecifier << "\n";
      }

      // instead of "see if the name can be found in the full list of
      // paths" we want to find all paths that match this name.
      //
      // 'specifier' now corresponds to the real trigger-path name,
      // free of any decorations.
      string const& realname{specifier};
      auto const matches = regexMatch(trigger_path_specs, realname);
      if (matches.empty()) {
        if (is_glob(realname)) {
          mf::LogWarning("Configuration")
            << "EventSelector::init, A module is using SelectEvents\n"
               "to request a wildcarded trigger name that "
               "does not match any trigger \n"
               "The wildcarded trigger name is: "
            << realname
            << " (from trigger-path specification: " << pathSpecifier << ") \n";
        } else {
          throw Exception(errors::Configuration)
            << "EventSelector::init, A module is using SelectEvents\n"
               "to request a trigger name that does not exist\n"
               "The unknown trigger name is: "
            << realname
            << " (from trigger-path specification: " << pathSpecifier << ") \n";
        }
      }

      auto makeBitInfoPass = [&trigger_path_specs](auto m) {
        return BitInfo{path_position(trigger_path_specs, m), true};
      };
      auto makeBitInfoFail = [&trigger_path_specs](auto m) {
        return BitInfo{path_position(trigger_path_specs, m), false};
      };

      if (!negative_criterion && !noex_demanded && !exception_spec) {
        cet::transform_all(
          matches, back_inserter(absolute_acceptors_), makeBitInfoPass);
        continue;
      }

      if (!negative_criterion && noex_demanded) {
        cet::transform_all(
          matches, back_inserter(conditional_acceptors_), makeBitInfoPass);
        continue;
      }

      if (exception_spec) {
        cet::transform_all(
          matches, back_inserter(exception_acceptors_), makeBitInfoPass);
        continue;
      }

      if (negative_criterion && !noex_demanded) {
        if (matches.empty()) {
          throw Exception(errors::Configuration)
            << "EventSelector::init, A module is using SelectEvents\n"
               "to request all fails on a set of trigger names that do not "
               "exist\n"
            << "The problematic name is: " << pathSpecifier << "\n";
        }

        if (matches.size() == 1) {
          BitInfo bi{path_position(trigger_path_specs, matches[0]), false};
          absolute_acceptors_.push_back(bi);
        } else {
          vector<BitInfo> mustfail;
          // We set this to false because that will demand bits are Fail.
          cet::transform_all(matches, back_inserter(mustfail), makeBitInfoFail);
          all_must_fail_.push_back(mustfail);
        }
        continue;
      }

      if (negative_criterion && noex_demanded) {
        if (matches.empty()) {
          throw Exception(errors::Configuration)
            << "EventSelector::init, A module is using SelectEvents\n"
               "to request all fails on a set of trigger names that do not "
               "exist\n"
            << "The problematic name is: " << pathSpecifier << "\n";
        }

        if (matches.size() == 1) {
          BitInfo bi{path_position(trigger_path_specs, matches[0]), false};
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
    auto const trigger_path_specs =
      pset.get<vector<string>>("trigger_paths", {});
    if (trigger_path_specs.size() != tr.size()) {
      throw Exception(errors::Unknown)
        << "EventSelector::acceptEvent: Trigger names vector and\n"
        << "TriggerResults are different sizes.  This should be impossible,\n"
        << "please send information to reproduce this problem to\n"
        << "the art developers.\n";
    }

    init(pathspecs_, trigger_path_specs);
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

    if (acceptOneBit(absolute_acceptors_, tr)) {
      return true;
    }

    bool exceptionPresent = false;
    bool exceptionsLookedFor = false;
    if (acceptOneBit(conditional_acceptors_, tr)) {
      exceptionPresent = tr.error();
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
          exceptionPresent = tr.error();
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
    bool const check_for_exception = (s == hlt::Exception);
    return std::any_of(
      begin(bits), end(bits), [check_for_exception, &tr](auto const& b) {
        hlt::HLTState const bstate = check_for_exception ?
                                       hlt::Exception :
                                       b.accept_state ? hlt::Pass : hlt::Fail;
        return tr.at(b.pos).state() == bstate;
      });
  }

  // Indicate if *every* bit in the trigger results matches the desired value
  // at that position, based on the bits array: true-->Pass, false-->Fail.
  bool
  EventSelector::acceptAllBits(vector<BitInfo> const& bits,
                               HLTGlobalStatus const& tr) const
  {
    return std::all_of(begin(bits), end(bits), [&tr](auto const& b) {
      hlt::HLTState const bstate = b.accept_state ? hlt::Pass : hlt::Fail;
      return tr.at(b.pos).state() == bstate;
    });
  }

} // namespace art
