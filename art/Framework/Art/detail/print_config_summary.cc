#include "art/Framework/Art/detail/print_config_summary.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "art/Persistency/Provenance/PathSpec.h"
#include "cetlib/HorizontalRule.h"
#include "fhiclcpp/ParameterSet.h"

#include <iomanip>
#include <iostream>

namespace {
  constexpr cet::HorizontalRule header_rule{50};
}

//---------------------------------------------------------------
void
art::detail::print_table_numbers(fhicl::ParameterSet const& pset,
                                 std::string const& header)
{
  auto const names = pset.get_names();
  std::cout << header << ": " << size(names) << '\n';
}

void
art::detail::print_path_numbers(EnabledModules const& enabled_modules)
{
  std::cout << "Trigger paths: " << size(enabled_modules.trigger_path_specs())
            << '\n'
            << (enabled_modules.trigger_paths_override() ?
                  " -> 'trigger_paths' specified\n" :
                  "")
            << "End paths: " << size(enabled_modules.end_paths()) << '\n'
            << (enabled_modules.end_paths_override() ?
                  " -> 'end_paths' specified\n" :
                  "");
}

void
art::detail::print_path_names(EnabledModules const& enabled_modules)
{
  {
    auto const& trigger_paths = enabled_modules.trigger_path_specs();
    std::cout << '\n'
              << header_rule('=') << '\n'
              << "Trigger paths: " << size(trigger_paths) << "\n"
              << (enabled_modules.trigger_paths_override() ?
                    " -> 'trigger_paths' specified\n\n" :
                    "\n");
    if (empty(trigger_paths)) {
      return;
    }

    std::string column_0{"Bit"};
    std::string column_1{"Path name"};
    std::string column_2{"Path size"};

    auto column_1_width = size(column_1);
    for (auto const& pr : trigger_paths) {
      column_1_width = std::max(column_1_width, size(pr.first.name));
    }

    std::cout << column_0 << "  " << std::left << std::setw(column_1_width)
              << column_1 << "  " << column_2 << '\n'
              << "---"
              << "  " << std::string(column_1_width, '-') << "  "
              << std::string(size(column_2), '-') << '\n';
    for (auto const& [path_spec, entries] :
         enabled_modules.trigger_path_specs()) {
      std::cout << std::right << std::setw(3) << to_string(path_spec.path_id)
                << "  " << std::left << std::setw(column_1_width)
                << path_spec.name << "  " << size(entries) << '\n';
    }
  }

  {
    auto const& end_paths = enabled_modules.end_paths();

    std::cout << '\n'
              << header_rule('=') << '\n'
              << "End paths: " << size(end_paths) << '\n'
              << (enabled_modules.end_paths_override() ?
                    " -> 'end_paths' specified\n\n" :
                    "\n");

    if (empty(end_paths)) {
      return;
    }

    std::string column_1{"Path name"};
    std::string column_2{"Path size"};
    auto column_1_width = size(column_1);
    for (auto const& pr : end_paths) {
      column_1_width = std::max(column_1_width, size(pr.first.name));
    }

    std::cout << std::left << std::setw(column_1_width) << column_1 << "  "
              << column_2 << '\n'
              << std::string(column_1_width, '-') << "  "
              << std::string(size(column_2), '-') << '\n';
    for (auto const& [path_spec, entries] : end_paths) {
      std::cout << std::left << std::setw(column_1_width) << path_spec.name
                << "  " << size(entries) << '\n';
    }
  }
}

void
art::detail::print_service_types(fhicl::ParameterSet const& pset)
{
  std::cout << '\n' << header_rule('=') << '\n';
  print_table_numbers(pset, "Services");
  std::cout << "\n";

  auto const names = pset.get_names();

  std::string column_1{"Service name"};
  auto column_1_width = size(column_1);
  auto column_2_width = std::string::size_type{};
  for (auto const& name : names) {
    auto const service_provider =
      pset.get<std::string>(fhicl_key(name, "service_provider"), {});
    column_1_width = std::max(column_1_width, size(name));
    column_2_width = std::max(column_2_width, size(service_provider));
  }

  cet::HorizontalRule const rule_1{column_1_width};
  if (column_2_width == 0ull) {
    std::cout << column_1 << '\n' << rule_1('-') << '\n';
    for (auto const& name : names) {
      std::cout << std::left << std::setw(column_1_width) << name << '\n';
    }
  } else {
    std::string const column_2{"service_provider"};
    cet::HorizontalRule const rule_2{std::max(column_2_width, size(column_2))};
    std::cout << std::left << std::setw(column_1_width) << column_1 << "  "
              << column_2 << '\n'
              << rule_1('-') << "  " << rule_2('-') << '\n';
    for (auto const& name : names) {
      auto const service_provider =
        pset.get<std::string>(fhicl_key(name, "service_provider"), "-");
      std::cout << std::left << std::setw(column_1_width) << name << "  "
                << service_provider << '\n';
    }
  }
}

void
art::detail::print_module_types(fhicl::ParameterSet const& pset,
                                std::string const& header)
{
  std::cout << '\n' << header_rule('=') << "\n";
  print_table_numbers(pset, header);

  auto const names = pset.get_names();
  if (empty(names)) {
    return;
  }

  std::cout << "\n";
  std::string column_1{"Module label"};
  std::string column_2{"module_type"};
  auto column_1_width = size(column_1);
  auto column_2_width = size(column_2);
  for (auto const& name : names) {
    auto const module_type =
      pset.get<std::string>(fhicl_key(name, "module_type"));
    column_1_width = std::max(column_1_width, size(name));
    column_2_width = std::max(column_2_width, size(module_type));
  }

  cet::HorizontalRule const rule_1{column_1_width};
  cet::HorizontalRule const rule_2{column_2_width};
  std::cout << std::left << std::setw(column_1_width) << column_1 << "  "
            << column_2 << '\n'
            << rule_1('-') << "  " << rule_2('-') << '\n';
  for (auto const& name : names) {
    auto const module_type =
      pset.get<std::string>(fhicl_key(name, "module_type"));
    std::cout << std::left << std::setw(column_1_width) << name << "  "
              << module_type << '\n';
  }
}

void
art::detail::print_config_summary(fhicl::ParameterSet const& pset,
                                  std::string const& verbosity,
                                  EnabledModules const& enabled_modules)
{
  auto const process_name = pset.get<std::string>("process_name");
  auto const source = pset.get<std::string>("source.module_type");
  auto const services = pset.get<fhicl::ParameterSet>("services");
  auto const physics = pset.get<fhicl::ParameterSet>("physics", {});
  auto const analyzers = physics.get<fhicl::ParameterSet>("analyzers", {});
  auto const filters = physics.get<fhicl::ParameterSet>("filters", {});
  auto const producers = physics.get<fhicl::ParameterSet>("producers", {});
  auto const outputs = pset.get<fhicl::ParameterSet>("outputs", {});

  std::cout << "Process name: " << process_name << '\n'
            << "Source module: " << source << '\n';
  if (verbosity == "brief") {
    print_table_numbers(services, "Services");
    print_table_numbers(producers, "Producers");
    print_table_numbers(filters, "Filters");
    print_table_numbers(analyzers, "Analyzers");
    print_table_numbers(outputs, "Output modules");
    print_path_numbers(enabled_modules);
    return;
  } else if (verbosity == "detailed") {
    print_service_types(services);
    std::cout << '\n' << header_rule('=') << '\n' << "Physics modules\n\n";
    print_table_numbers(producers, "Producers");
    print_table_numbers(filters, "Filters");
    print_table_numbers(analyzers, "Analyzers");
    print_module_types(outputs, "Output modules");
    print_path_names(enabled_modules);
    return;
  } else if (verbosity == "full") {
    print_service_types(services);
    print_module_types(producers, "Producers");
    print_module_types(filters, "Filters");
    print_module_types(analyzers, "Analyzers");
    print_module_types(outputs, "Output modules");
    print_path_names(enabled_modules);
    return;
  }
  throw art::Exception{art::errors::Configuration}
    << "Unrecognized configuration-summary verbosity level: '" << verbosity
    << "'\n";
}
