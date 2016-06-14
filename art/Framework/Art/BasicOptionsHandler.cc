#include "art/Framework/Art/BasicOptionsHandler.h"

#include "art/Framework/Art/detail/PrintPluginMetadata.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/PluginSuffixes.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/filepath_maker.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <iostream>
#include <string>

using namespace std::string_literals;

namespace {

  using table_t = fhicl::extended_value::table_t;

} // namespace

art::BasicOptionsHandler::
BasicOptionsHandler(bpo::options_description & desc,
                    cet::filepath_maker & maker)
  :
  help_desc_(desc),
  maker_(maker)
{
  desc.add_options()
    ("config,c", bpo::value<std::string>(), "Configuration file.")
    ("help,h", "produce help message")
    ("process-name", bpo::value<std::string>(), "art process name.")
    ("print-available", bpo::value<std::string>(),
     ("List all available plugins with the provided suffix.  Choose from:"s + Suffixes::print()).c_str())
    ("print-available-modules",
     "List all available modules that can be invoked in a FHiCL file.")
    ("print-available-services",
     "List all available services that can be invoked in a FHiCL file.")
    ("print-description",bpo::value<std::vector<std::string>>()->multitoken(),
     "Print description of specified module, service, source, or other plugin (multiple OK).")
    ;
}

int
art::BasicOptionsHandler::
doCheckOptions(bpo::variables_map const & vm)
{
  // Technically the "help" and "print*" options are processing steps,
  // but we want to short-circuit.
  if (vm.count("help")) {
    std::cout << help_desc_ << std::endl; // Note NOT our own desc_.
    return 1;
  }
  if ( vm.count("print-available") ) {
    detail::print_available_plugins(Suffixes::get(vm["print-available"].as<std::string>()));
    return 1;
  }
  if ( vm.count("print-available-modules") ) {
    detail::print_available_plugins(suffix_type::module);
    return 1;
  }
  if ( vm.count("print-available-services") ) {
    detail::print_available_plugins(suffix_type::service);
    return 1;
  }
  if ( vm.count("print-description") ) {
    detail::print_descriptions( vm["print-description"].as<std::vector<std::string>>());
    return 1;
  }

  if (!vm.count("config")) {
    throw Exception(errors::Configuration)
        << "No configuration file given.\n";
  }
  return 0;
}

int
art::BasicOptionsHandler::
doProcessOptions(bpo::variables_map const & vm,
                 fhicl::intermediate_table & raw_config)
{
  try {
    fhicl::parse_document(vm["config"].as<std::string>(), maker_, raw_config);
  }
  catch (cet::exception & e) {
    std::cerr << "Failed to parse the configuration file '"
              << vm["config"].as<std::string>()
              << "' with exception\n" << e.what()
              << "\n";
    return 90;
  }
  if (raw_config.empty()) {
    std::cerr << "INFO: provided configuration file '"
              << vm["config"].as<std::string>()
              << "' is empty: \n"
              << "using minimal defaults and command-line options.\n";
  }
  if (vm.count("process-name")) {
    raw_config.put("process_name",
                   vm["process-name"].as<std::string>());
  }
  return 0;
}
