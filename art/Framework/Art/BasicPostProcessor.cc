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
  verifyInterfaces(fhicl::intermediate_table & raw_config)
  {
    if (!raw_config.exists("services.user.CatalogInterface.service_provider")) {
      raw_config.put("services.user.CatalogInterface.service_provider",
                     "TrivialFileDelivery");
    }
    if (!raw_config.exists("services.user.FileTransfer.service_provider")) {
      raw_config.put("services.user.FileTransfer.service_provider",
                     "TrivialFileTransfer");
    }
  }

  void
  verifySourceConfig(fhicl::intermediate_table & raw_config)
  {
    if (raw_config.exists("source.fileNames")) {
      if (raw_config.exists("source.module_type")) {
        if (raw_config.get<std::string>("source.module_type") == "EmptyEvent") {
          throw art::Exception(art::errors::Configuration)
            << "Error: source files specified for EmptyEvent source.";
        }
      }
      else {
        raw_config.put("source.module_type", "RootInput");
      }
    } else if (!raw_config.exists("source.module_type")) {
      raw_config.put("source.module_type", "EmptyEvent");
    }
    if (raw_config.get<std::string>("source.module_type") == "EmptyEvent" &&
        !raw_config.exists("source.maxEvents")) {
      // Default 1 event.
      raw_config.put("source.maxEvents", 1);
    }
  }

  void
  injectModuleLabels(fhicl::intermediate_table & int_table,
                     std::string const & table_spec)
  {
    if (!int_table.exists(table_spec)) { return; }
    auto & top_table_val = int_table[table_spec];
    if (!top_table_val.is_a(fhicl::TABLE)) {
      throw art::Exception(art::errors::Configuration)
        << "Unexpected non-table "
        << table_spec
        << ".\n";
    }
    auto & table = int_table.get<table_t &>(table_spec);
    for (auto const & tval : table) {
      if (tval.first.find('_') != std::string::npos) {
        throw art::Exception(art::errors::Configuration)
          << "Module parameter set label \""
          << tval.first
          << "\" is illegal: "
          << "underscores are not permitted in module names.";
      }
      auto & table_val = int_table[table_spec + '.' + tval.first];
      if (!table_val.is_a(fhicl::TABLE)) {
        throw art::Exception(art::errors::Configuration)
          << "Unexpected non-table "
          << tval.first
          << " found in "
          << table_spec
          << ".\n";
      };
      int_table.put(table_spec + '.' + tval.first + ".module_label",
                    tval.first);
    }
  }

  void
  addModuleLabels(fhicl::intermediate_table & raw_config)
  {
    if (raw_config.exists("source")) {
      raw_config.put("source.module_label", "source");
    }
    injectModuleLabels(raw_config, "outputs");
    injectModuleLabels(raw_config, "physics.producers");
    injectModuleLabels(raw_config, "physics.filters");
    injectModuleLabels(raw_config, "physics.analyzers");
  }

  void injectServiceType(fhicl::intermediate_table & raw_config,
                         std::string const & table_spec,
                         std::vector<std::string> const & excluded = { })
  {
    if (!raw_config.exists(table_spec)) { return; }
    auto & top_table_val = raw_config[table_spec];
    if (!top_table_val.is_a(fhicl::TABLE)) {
      throw art::Exception(art::errors::Configuration)
        << "Unexpected non-table "
        << table_spec
        << ".\n";
    }
    auto & table = raw_config.get<table_t &>(table_spec);
    for (auto const & tval : table) {
      auto & table_val = raw_config[table_spec + '.' + tval.first];
      if (!table_val.is_a(fhicl::TABLE)) {
        throw art::Exception(art::errors::Configuration)
          << "Unexpected non-table "
          << tval.first
          << " found in "
          << table_spec
          << ".\n";
      };
      if (std::find(excluded.cbegin(), excluded.cend(), tval.first) ==
          excluded.end()) {
        raw_config.put(table_spec + '.' + tval.first + ".service_type",
                       tval.first);
      }
    }
  }

  void addServiceType(fhicl::intermediate_table & raw_config)
  {
    std::vector<std::string> const excluded =
      { "message", "scheduler", "user" };
    injectServiceType(raw_config, "services", excluded);
    injectServiceType(raw_config, "services.user");
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
  verifyInterfaces(raw_config);
  verifySourceConfig(raw_config);
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
  // module_labels and service_type.
  addModuleLabels(raw_config);
  addServiceType(raw_config);
  return 0; // If anything had gone wrong, we would have thrown.
}
