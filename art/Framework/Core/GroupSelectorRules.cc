#include "art/Framework/Core/GroupSelectorRules.h"

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/detail/branchNameComponentChecking.h"
#include "canvas/Utilities/Exception.h"
#include "boost/algorithm/string.hpp"
#include "cetlib/container_algorithms.h"

#include <algorithm>
#include <cctype>
#include <ostream>
#include <regex>
#include <string>

using namespace art;
using namespace cet;
using namespace fhicl;
using namespace std;

using VCBDMP = vector<BranchDescription const*>;

namespace {

  // The partial_match() functions are helpers for Rule().  They
  // ascertain matches between criterion and candidate value for
  // components of the branch description, with appropriate wildcard
  // rules.
  inline
  bool
  partial_match(string const& regularExpression,
                string const& branchstring)
  {
    return regularExpression.empty()
      ? branchstring == ""
      : std::regex_match(branchstring, std::regex(regularExpression) );
  }

  inline
  bool
  partial_match(art::BranchType wanted,
                art::BranchType candidate)
  {
    bool result = (wanted == art::NumBranchTypes) || (wanted == candidate);
    return result;
  }

  using namespace std::string_literals;
  static auto const branchTypeString = "[Ii]n(?:(Event)|(SubRun)|(Run)|(Results))"s;
  static std::regex const branchTypeRE(branchTypeString);

  static std::string const rulesMsg =
    "Syntax: keep|drop <spec> [<branchtype]>\n"
    "where <spec> is EITHER \"*\" OR:\n"
    "<friendly-type>_<module-label>_<instance-name>_<process-name>\n"
    "Wildcards are permissible within each field: * (any number of characters), or\n"
    "? (any single permissible character).\n"
    "Permissible non-wildcard characters in all fields: [A-Za-z0-9].\n"
    "Additionally, \"::\" is permissible in friendly type names, and\n"
    "\"#\" is permissible in module labels.\n";

  art::BranchKey parseComponents(std::string s,
                                 std::string const & parameterName,
                                 std::string const & owner,
                                 bool & selectflag)
  {
    BranchKey components;
    std::smatch ruleMatch;
    static std::regex const
      re("(keep|drop)\\s+(\\*|(?:[^_]*)_(?:[^_]*)_(?:[^_]*)_(?:[^_\\s]*))(?:\\s+(.*))?");
    boost::trim(s); // Removing leading / trailing whitespace.
    if (!std::regex_match(s, ruleMatch, re)) { // Failed preliminary check.
      throw Exception(errors::Configuration)
        << "Illegal product selection rule \""
        << s << "\" failed initial checks in "
        << owner << '.' << parameterName << ".\n"
        << rulesMsg;
    }
    selectflag = (ruleMatch[1].str() == "keep");
    if (ruleMatch[2].str() == "*") { // special case for wildcard
      components.friendlyClassName_  = ".*";
      components.moduleLabel_  = ".*";
      components.productInstanceName_ = ".*";
      components.processName_  = ".*";
    } else {
      std::string errMsg;

      components = art::detail::splitToComponents(ruleMatch[2], errMsg);

      if (!errMsg.empty()) {
        throw Exception(errors::Configuration)
          << errMsg
          << "Error occurred in "
          << owner << '.' << parameterName
          << " (exactly four components required if not \"*\").\n"
          << rulesMsg;
      }

      bool good = art::detail::checkBranchNameSelector(components, errMsg);

      if (!good) {
        throw Exception(errors::Configuration)
          << errMsg
          << "Error occurred in "
          << owner << '.' << parameterName << ".\n"
          << rulesMsg;
      }
      boost::replace_all(components.friendlyClassName_, "*", ".*");
      boost::replace_all(components.friendlyClassName_, "?", ".");
      boost::replace_all(components.moduleLabel_, "*", ".*");
      boost::replace_all(components.moduleLabel_, "?", ".");
      boost::replace_all(components.productInstanceName_, "*", ".*");
      boost::replace_all(components.productInstanceName_, "?", ".");
      boost::replace_all(components.processName_, "*", ".*");
      boost::replace_all(components.processName_, "?", ".");
    }
    if ((ruleMatch[3].length() > 0) && // Have a BranchType specification.
        (ruleMatch[3] != "*")) { // Wildcard is NOP, here.
      std::smatch btMatch;
      auto const foundBT = ruleMatch[3].str();
      if (std::regex_match(foundBT,
                           btMatch,
                           branchTypeRE)) {
        // Should be true by construction.
        assert(btMatch.size() == static_cast<size_t>(art::NumBranchTypes) + 1ul);
        auto itFirstMatch = btMatch.begin();
        ++itFirstMatch;
        auto it = std::find_if(itFirstMatch, btMatch.end(), [](auto & s){ return (s.length() > 0); });
        assert(it != btMatch.end()); // Should be true by construction.
        components.branchType_ = std::distance(itFirstMatch, it);
      } else {
        throw art::Exception(art::errors::Configuration)
          << "Invalid branch type specification \""
          << ruleMatch[3]
          << "\" in " << owner << " parameter named '" << parameterName << "'\n"
          "If the optional branch type is specified, it must satisfy the following regex: \"^"
          << branchTypeString
          << "$\".\n"
          << rulesMsg;
      }
    }
    return components;
  }

}

GroupSelectorRules::Rule::Rule(string const& s,
                               string const& parameterName,
                               string const& owner) :
  selectflag_(),
  components_(parseComponents(s, parameterName, owner, selectflag_))
{
}

void
GroupSelectorRules::Rule::applyToAll(vector<BranchSelectState>& branchstates) const
{
  for (auto& state : branchstates)
    applyToOne(state.desc, state.selectMe);
}

void
GroupSelectorRules::applyToAll(vector<BranchSelectState>& branchstates) const
{
  for (auto const& rule : rules_)
    rule.applyToAll(branchstates);
}

void
GroupSelectorRules::Rule::applyToOne(BranchDescription const* branch,
                                     bool& result) const
{
  if (this->appliesTo(branch)) result = selectflag_;
}

bool
GroupSelectorRules::Rule::appliesTo(BranchDescription const* branch) const
{
  return
    partial_match(components_.friendlyClassName_ , branch->friendlyClassName()) &&
    partial_match(components_.moduleLabel_ , branch->moduleLabel()) &&
    partial_match(components_.productInstanceName_, branch->productInstanceName()) &&
    partial_match(components_.processName_ , branch->processName()) &&
    partial_match(static_cast<BranchType>(components_.branchType_), branch->branchType());
}

GroupSelectorRules::GroupSelectorRules(vector<string> const & commands,
                                       string const& parameterName,
                                       string const& parameterOwnerName) :
  rules_()
{
  rules_.reserve(commands.size());
  for(auto const& cmd : commands) {
    rules_.push_back(Rule(cmd, parameterName, parameterOwnerName));
  }
  keepAll_ = commands.size() == 1 && commands[0] == "keep *";
}

// ======================================================================
