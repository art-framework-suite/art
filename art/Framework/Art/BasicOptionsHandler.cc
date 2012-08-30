#include "art/Framework/Art/BasicOptionsHandler.h"

#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/filepath_maker.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <iostream>
#include <string>

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
  ;
}

int
art::BasicOptionsHandler::
doCheckOptions(bpo::variables_map const & vm)
{
  if (vm.count("help")) {
    // Technically a processing step, but we want to short-circuit.
    std::cout << help_desc_ << std::endl; // Note NOT our own desc_.
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
    return 7002;
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
