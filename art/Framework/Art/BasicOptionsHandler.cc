#include "art/Framework/Art/BasicOptionsHandler.h"
#include "art/Framework/Core/detail/PrintAvailablePlugins.h"

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
    ("print-available-modules",
     "List all available modules that can be invoked in a FHiCL file")
    ("print-available-services",
     "List all available services that can be invoked in a FHiCL file")
    ("module-description",bpo::value<std::vector<std::string>>()->multitoken(),
     "Print description of a module, specified by the 'module_type' value "
     "(can accept multiple module specs).")
    ("service-description",bpo::value<std::vector<std::string>>()->multitoken(),
     "Print description of a services, as specified in the 'services' block of a user's FHiCL file "
     "(can accept multiple service names).")
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
  if ( vm.count("print-available-modules") ){
    detail::print_available_modules();
    return 1;
  }
  if ( vm.count("print-available-services") ) {
    detail::print_available_services();
    return 1;
  }
  if ( vm.count("module-description") ) {
    detail::print_module_description( vm["module-description"].as<std::vector<std::string>>() );
    return 1;
  }
  if ( vm.count("service-description") ) {
    detail::print_service_description( vm["service-description"].as<std::vector<std::string>>() );
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
