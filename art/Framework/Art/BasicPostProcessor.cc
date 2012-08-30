#include "art/Framework/Art/BasicPostProcessor.h"

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

  void
  verifyProcessName(fhicl::intermediate_table & raw_config)
  {
    if (raw_config.exists("process_name")) {
      auto process_name(raw_config.get<std::string>("process_name"));
      if (process_name.empty()) {
        throw cet::exception("BAD_PROCESS_NAME")
            << "Empty process_name not permitted.";
      }
      else if (process_name.find('_') != std::string::npos) {
        throw cet::exception("BAD_PROCESS_NAME")
            << "Underscores not permitted in process_name: illegal value \""
            << process_name
            << "\"";
      }
      else {
        ;
      }
    }
    else {
      std::cerr << "INFO: using default process_name, \"DUMMY.\"\n";
      raw_config.put("process_name", "DUMMY");
    }
  }

  void
  injectModuleLabels(fhicl::intermediate_table & int_table,
                     std::string const & table_spec,
                     std::vector<std::string> & all_modules)
  {
    if (!int_table.exists(table_spec)) { return; }
    auto & top_table_val = int_table[table_spec];
    if (!top_table_val.is_a(fhicl::TABLE)) { return; }
    auto & table = int_table.get<table_t &>(table_spec);
  for (auto const & tval : table) {
      if (tval.first.find('_') != std::string::npos) {
        throw cet::exception("BAD_MODULE_LABEL")
            << "Module parameter set label \""
            << tval.first
            << "\" is illegal: "
            << "underscores are not permitted in module names.";
      }
      int_table.put(table_spec + '.' + tval.first + ".module_label",
                    tval.first);
      all_modules.push_back(tval.first);
    }
  }

  int
  addModuleLabels(fhicl::intermediate_table & raw_config)
  {
    if (raw_config.exists("source")) {
      raw_config.put("source.module_label", "source");
    }
    std::vector<std::string> all_modules;
    injectModuleLabels(raw_config, "outputs", all_modules);
    injectModuleLabels(raw_config, "physics.producers", all_modules);
    injectModuleLabels(raw_config, "physics.filters", all_modules);
    injectModuleLabels(raw_config, "physics.analyzers", all_modules);
    raw_config.put("all_modules", all_modules);
    return 0;
  }

} // namespace

int
art::BasicPostProcessor::
doCheckOptions(bpo::variables_map const &)
{
  return 0; // NOP
}

int
art::BasicPostProcessor::
doProcessOptions(bpo::variables_map const &,
                 fhicl::intermediate_table & raw_config)
{
  verifyProcessName(raw_config);
  // trigger_paths
  if (raw_config.exists("physics.trigger_paths")) {
    raw_config["trigger_paths.trigger_paths"] =
      raw_config["physics.trigger_paths"];
  }
  // messagefacility configuration.
  if (!raw_config.exists("services.message")) {
    raw_config.put("services.message.destinations.STDOUT.categories.ArtReport.limit", 100);
    raw_config.put("services.message.destinations.STDOUT.categories.default.limit", -1);
    raw_config.put("services.message.destinations.STDOUT.type", "cout");
    raw_config.put("services.message.destinations.STDOUT.threshold", "INFO");
  }
  // module_labels and all_modules.
  return addModuleLabels(raw_config);
}
