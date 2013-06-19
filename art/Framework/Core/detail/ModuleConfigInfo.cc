#include "art/Framework/Core/detail/ModuleConfigInfo.h"

#include "art/Utilities/Exception.h"

#include <algorithm>
#include <string>

art::detail::ModuleConfigInfo::
ModuleConfigInfo(fhicl::ParameterSet const & procPS,
                 std::string const & label,
                 std::string const & configPath)
  :
  label_(label),
  configPath_(configPath),
  filterAction_(calcFilterAction_()),
  moduleType_(calcConfigType_()),
  modPS_(procPS.get<fhicl::ParameterSet>(configPath_ +
                                  '.' +
                                  label_)),
  libSpec_(modPS_.get<std::string>("module_type"))
{
}

std::vector<std::string> const &
art::detail::ModuleConfigInfo::
allModulePathRoots()
{
  static const std::vector<std::string> s_allModulePathRoots = {
    "physics.analyzers",
    "physics.filters",
    "outputs",
    "physics.producers"
  };
  return s_allModulePathRoots;
}

std::string
art::detail::ModuleConfigInfo::
stripLabel(std::string const & labelInPathConfig)
{
  auto label_start = labelInPathConfig.find_first_not_of("!-");
  if (label_start > 1) {
    throw Exception(errors::Configuration)
        << "Module label "
        << labelInPathConfig
        << " is illegal.\n";
  }
  return labelInPathConfig.substr(label_start);
}

// Called from initializer: verify initialization order with member data
// use if altering this function!
art::WorkerInPath::FilterAction
art::detail::ModuleConfigInfo::
calcFilterAction_() const
{
  if (label_.empty()) {
    throw Exception(errors::Configuration)
        << "Empty module label in path "
        << configPath_
        << ".\n";
  }
  switch (label_[0]) {
    case '!':
      return WorkerInPath::FilterAction::Veto;
    case '-':
      return WorkerInPath::FilterAction::Ignore;
    default:
      return WorkerInPath::FilterAction::Normal;
  }
}

// Called from initializer: verify initialization order with member data
// use if altering this function!
art::ModuleType
art::detail::ModuleConfigInfo::
calcConfigType_() const
{
  auto const & paths = allModulePathRoots();
  switch (std::find(paths.cbegin(), paths.cend(), configPath_) -
          paths.cbegin()) {
    case 0:
      return ModuleType::ANALYZER;
    case 1:
      return ModuleType::FILTER;
    case 2:
      return ModuleType::OUTPUT;
    case 3:
      return ModuleType::PRODUCER;
    default:
      throw Exception(errors::LogicError)
          << "Unrecognized module path "
          << configPath_
          << ".\n";
  };
}
