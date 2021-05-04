#include "art/Framework/Art/BasicOptionsHandler.h"

#include "art/Framework/Art/detail/AllowedConfiguration.h"
#include "art/Framework/Art/detail/info_success.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/filepath_maker.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <iostream>
#include <string>

using namespace std::string_literals;
using table_t = fhicl::extended_value::table_t;
namespace {

  std::string
  pretty_version(std::string s)
  {
    std::replace(s.begin(), s.end(), '_', '.');
    return s.substr(1); // trim off 'v'
  }

  auto
  to_string(bool const value)
  {
    return value ? "true"s : "false"s;
  }

} // namespace

art::BasicOptionsHandler::BasicOptionsHandler(bpo::options_description& desc,
                                              cet::filepath_maker& maker,
                                              bool const report_unused)
  : help_desc_{desc}, maker_{maker}
{
  // clang-format off
  desc.add_options()
    ("help,h", "produce help message")
    ("version", ("Print art version (" + pretty_version(art::getReleaseVersion()) + ")")
      .c_str())
    ("config,c", bpo::value<std::string>(), "Configuration file.")
    ("process-name", bpo::value<std::string>(), "art process name.")
    ("prune-config",
       bpo::value<bool>()->default_value(true, to_string(true)),
       "Remove unused modules and paths from the fully-processed configuration.")
    ("report-unused",
       bpo::value<bool>()->default_value(report_unused, to_string(report_unused)),
       "If 'true', the list of unused modules and paths will be printed to STDERR.")
    ("print-available",
       bpo::value<std::string>(),
       ("List all available plugins with the provided suffix.  Choose from:"s +
        Suffixes::print()).c_str())
    ("print-available-modules",
       "List all available modules that can be invoked in a FHiCL file.")
    ("print-available-services",
       "List all available services that can be invoked in a FHiCL file.")
    ("print-description",
       bpo::value<std::vector<std::string>>()->multitoken(),
       "Print description of specified module, service, source, or other "
       "plugin (multiple OK).  Argument can be a regular expression used "
       "to match the plugin specification.  To narrow the search to "
       "plugins with a particular suffix, preface the regular expression"
       "with the suffix (e.g. service:TFileService).")
    ("status-bar",
       "Provide status bar that reports the progress of retrieving "
       "plugin information for a 'print-available' command.");
  // clang-format on
}

int
art::BasicOptionsHandler::doCheckOptions(bpo::variables_map const& vm)
{
  // Technically the "help" and "print*" options are processing steps,
  // but we want to short-circuit.
  if (vm.count("help")) {
    // Could simply do cout << help_desc_, but the boost-provided
    // printout does not add any left-hand padding.  Will add a
    // 2-space tab by hand.
    std::stringstream ss;
    ss << help_desc_; // Note NOT our own desc_.
    for (std::string s; std::getline(ss, s);)
      std::cout << std::string(2, ' ') << s << '\n';
    std::cout << '\n';
    return detail::info_success();
  }
  bool const status_bar = vm.count("status-bar") > 0;
  if (vm.count("print-available")) {
    detail::print_available_plugins(vm["print-available"].as<std::string>(),
                                    status_bar);
    return detail::info_success();
  }
  if (vm.count("print-available-modules")) {
    detail::print_available_plugins(Suffixes::module(), status_bar);
    return detail::info_success();
  }
  if (vm.count("print-available-services")) {
    detail::print_available_plugins(Suffixes::service(), status_bar);
    return detail::info_success();
  }
  if (status_bar) {
    throw Exception(errors::Configuration)
      << "The '--status-bar' option can be used only with the "
         "'--print-available*' program options.\n";
  }

  if (vm.count("print-description")) {
    detail::print_descriptions(
      vm["print-description"].as<std::vector<std::string>>());
    return detail::info_success();
  }
  if (vm.count("version")) {
    std::cout << "art " << pretty_version(getReleaseVersion()) << '\n';
    return detail::info_success();
  }

  if (!vm.count("config")) {
    throw Exception(errors::Configuration) << "No configuration file given.\n";
  }
  return 0;
}

int
art::BasicOptionsHandler::doProcessOptions(
  bpo::variables_map const& vm,
  fhicl::intermediate_table& raw_config)
{
  try {
    raw_config = fhicl::parse_document(vm["config"].as<std::string>(), maker_);
  }
  catch (cet::exception& e) {
    std::cerr << "Failed to parse the configuration file '"
              << vm["config"].as<std::string>() << "' with exception\n"
              << e.what() << "\n";
    return 90;
  }
  if (raw_config.empty()) {
    std::cerr << "INFO: provided configuration file '"
              << vm["config"].as<std::string>() << "' is empty: \n"
              << "using minimal defaults and command-line options.\n";
  }
  if (vm.count("process-name")) {
    raw_config.put("process_name", vm["process-name"].as<std::string>());
  }
  if (vm.count("prune-config")) {
    raw_config.put("services.scheduler.pruneConfig",
                   vm["prune-config"].as<bool>());
  }
  if (vm.count("report-unused")) {
    raw_config.put("services.scheduler.reportUnused",
                   vm["report-unused"].as<bool>());
  }
  return 0;
}
