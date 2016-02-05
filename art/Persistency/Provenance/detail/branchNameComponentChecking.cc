#include "canvas/Persistency/Provenance/detail/branchNameComponentChecking.h"

#include "canvas/Persistency/Provenance/BranchKey.h"

#include "boost/algorithm/string.hpp"

#include <regex>
#include <string>
#include <vector>

namespace {

  static std::regex const typeRE("^(?:[[:alnum:]]|::)*");
  static std::regex const typeSelectorRE("^(?:[[:alnum:]\\*\\?]|::)*");
  static std::regex const labelRE("^[[:alnum:]#]*");
  static std::regex const labelSelectorRE("^[[:alnum:]#\\*\\?]*");
  static std::regex const processRE("^[[:alnum:]]*");
  static std::regex const processSelectorRE("^[[:alnum:]\\*\\?]*");

  bool
  checkBranchNameComponent(std::string const & component,
                           std::string const & designation,
                           std::regex const & re,
                           std::string & errMsg,
                           bool emptyOK = false)
  {
    bool result = true;
    if (component.empty()) {
      if (!emptyOK) {
        errMsg += "Illegal empty " + designation + ".\n";
      }
    } else {
      std::smatch sm;
      result = std::regex_search(component, sm, re) && sm.suffix().str().empty();
      if (!result) {
        errMsg += "Illegal character(s) found in " + designation +
                  "\n  " + component + "\n" +
                  std::string(2ul + sm.length(), ' ') +
                  '^' + std::string(component.length() - sm.length(), ' ') +
                  "(first infraction marked).\n";
      }
    }
    return result;
  }
}

art::BranchKey
art::detail::splitToComponents(std::string const & branchName,
                               std::string & errMsg)
{
  std::vector<std::string> parts;
  boost::split(parts, branchName, boost::is_any_of("_"));
  if (parts.size() != 4) {
    errMsg += "Illegal product name specification \"" +
              branchName + "\".\n";
  }
  return BranchKey(std::move(parts[0]),
                   std::move(parts[1]),
                   std::move(parts[2]),
                   std::move(parts[3]));
}

bool
art::detail::
checkBranchNameSelector(std::string const & branchNameSelector,
                        std::string & errMsg)
{
  errMsg.clear();
  auto components = splitToComponents(branchNameSelector, errMsg);
  return (!errMsg.empty()) || checkBranchNameSelector(components, errMsg);
}

bool
art::detail::
checkBranchNameSelector(BranchKey const & components,
                        std::string & errMsg)
{
  // Inclusive operation: do all steps.
  bool result = checkFriendlyNameSelector(components.friendlyClassName_,
                                          errMsg);
  result = checkModuleLabelSelector(components.moduleLabel_,
                                    errMsg) && result;
  result = checkInstanceNameSelector(components.productInstanceName_,
                                     errMsg) && result;
  result = checkProcessNameSelector(components.processName_,
                                    errMsg) && result;
  return result;
}

bool
art::detail::
checkFriendlyName(std::string const & friendlyName,
                  std::string & errMsg)
{
  return checkBranchNameComponent(friendlyName,
                                  "friendly name",
                                  typeRE,
                                  errMsg);
}

bool
art::detail::
checkFriendlyNameSelector(std::string const & friendlyNameSelector,
                          std::string & errMsg)
{
  return checkBranchNameComponent(friendlyNameSelector,
                                  "friendly name",
                                  typeSelectorRE,
                                  errMsg);
}

bool
art::detail::
checkModuleLabel(std::string const & moduleLabel,
                 std::string & errMsg)
{
  return checkBranchNameComponent(moduleLabel,
                                  "module label",
                                  labelRE,
                                  errMsg);
}

bool
art::detail::
checkModuleLabelSelector(std::string const & moduleLabelSelector,
                         std::string & errMsg)
{
  return checkBranchNameComponent(moduleLabelSelector,
                                  "module label",
                                  labelSelectorRE,
                                  errMsg);
}

bool
art::detail::
checkInstanceName(std::string const & instanceName,
                  std::string & errMsg)
{
  return checkBranchNameComponent(instanceName,
                                  "instance name",
                                  processRE, // sic.
                                  errMsg,
                                  true);
}

bool
art::detail::
checkInstanceNameSelector(std::string const & instanceNameSelector,
                          std::string & errMsg)
{
  return checkBranchNameComponent(instanceNameSelector,
                                  "instance name",
                                  processSelectorRE, // sic.
                                  errMsg,
                                  true);
}

bool
art::detail::
checkProcessName(std::string const & processName,
                 std::string & errMsg)
{
  return checkBranchNameComponent(processName,
                                  "process name",
                                  processRE,
                                  errMsg);
}

bool
art::detail::
checkProcessNameSelector(std::string const & processNameSelector,
                         std::string & errMsg)
{
  return checkBranchNameComponent(processNameSelector,
                                  "process name",
                                  processSelectorRE,
                                  errMsg);
}
