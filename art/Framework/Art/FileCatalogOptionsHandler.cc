#include "art/Framework/Art/FileCatalogOptionsHandler.h"

#include "art/Utilities/Exception.h"
#include "art/Utilities/detail/serviceConfigLocation.h"
#include "art/Utilities/ensureTable.h"
#include "cetlib/split.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {

  using table_t = fhicl::extended_value::table_t;

  typedef std::pair<std::string const, std::string> string_pair_t;
  string_pair_t
  split_to_pair(std::string const & to_split)
  {
    std::vector<std::string> tmp;
    tmp.reserve(2);
    cet::split(to_split, ':', std::back_inserter(tmp));
    switch (tmp.size()) {
      case 0:
        return string_pair_t();
      case 1:
        return string_pair_t(std::string("_default"), std::move(tmp[0]));
      case 2:
        return string_pair_t(std::move(tmp[0]), std::move(tmp[1]));
      default:
        throw art::Exception(art::errors::Configuration)
            << "Expected \"key:value\", got multiple \":\".\n";
    }
  }

  void fill_tiers_streams(bpo::variables_map const & vm,
                          fhicl::intermediate_table & raw_config)
  {
    // Precondition: at least one output module defined in the
    // configuration.
    auto const & table = raw_config.get<table_t const &>("outputs");
    std::string const outputs_stem("outputs.");
    std::string const tier_spec_stem(".dataTier");
    std::string const stream_name_stem(".streamName");
    std::vector<std::string>
      data_tiers((vm.count("sam-data-tier") > 0) ?
                 vm["sam-data-tier"].as<std::vector<std::string> >() :
                 std::vector<std::string>());
    std::vector<std::string>
      stream_names((vm.count("sam-stream-name") > 0) ?
                   vm["sam-stream-name"].as<std::vector<std::string> >() :
                   std::vector<std::string>());
    std::map<std::string, std::string> sep_tiers, sep_streams;
    for (auto const & tier : data_tiers) {
      sep_tiers.insert(split_to_pair(tier));
    }
    for (auto const & stream : stream_names) {
      sep_streams.insert(split_to_pair(stream));
    }
    auto def_tier_it(sep_tiers.find("_default"));
    auto def_tier((def_tier_it != sep_tiers.end()) ?
                  def_tier_it->second :
                  "");
    auto def_stream_it(sep_streams.find("_default"));
    auto def_stream((def_stream_it != sep_streams.end()) ?
                    def_stream_it->second :
                    "");
    for (auto const & output : table) {
      if (!raw_config.exists(outputs_stem + output.first + ".module_type")) {
        continue; // Not a module parameter set.
      }
      auto tiers_it(sep_tiers.find(output.first));
      std::string tier((tiers_it != sep_tiers.end()) ?
                       tiers_it->second :
                       def_tier);
      if (!tier.empty()) {
        raw_config.put(outputs_stem + output.first + tier_spec_stem,
                       tier);
      }
      auto streams_it(sep_streams.find(output.first));
      std::string stream((streams_it != sep_streams.end()) ?
                         streams_it->second :
                         def_stream);
      raw_config.put(outputs_stem + output.first + stream_name_stem,
                     (!stream.empty()) ?
                     stream :
                     output.first);
      if (raw_config.get<std::string>(outputs_stem + output.first + ".module_type" ) == "RootOutput") {
        if (!(raw_config.exists(outputs_stem + output.first + tier_spec_stem)  &&
              raw_config.exists(outputs_stem + output.first + stream_name_stem))) {
          throw art::Exception(art::errors::Configuration)
          << "Output \""
          << output.first
          << "\" must be configured with "
          << tier_spec_stem.substr(1)
          << " (--sam-data-tier=" << output.first << ":<tier>) and "
          << stream_name_stem.substr(1)
          << " (--sam-stream-name=" << output.first << ":<stream>).\n";
        }
      }
    }
  }

  bool have_outputs(fhicl::intermediate_table & table) {
    bool result { false };
    if (table.exists("outputs")) {
      auto const & ev = table["outputs"];
      if (ev.is_a(fhicl::TABLE) &&
          ! table.get<fhicl::extended_value::table_t const &>("outputs").empty()) {
        result = true;
      }
    }
    return result;
  }

  void maybeThrowOnMissingMetadata(fhicl::intermediate_table const & table) {
    std::vector<std::string> missingItems;
    if (!table.exists("services.FileCatalogMetadata.applicationFamily")) {
      missingItems.emplace_back("services.FileCatalogMetadata.applicationFamily (--sam-application-family)");
    }
    if (!table.exists("services.FileCatalogMetadata.applicationVersion")) {
      missingItems.emplace_back("services.FileCatalogMetadata.applicationVersion (--sam-application-version)");
    }
    if (!table.exists("services.FileCatalogMetadata.group")) {
      missingItems.emplace_back("services.FileCatalogMetadata.group (--sam-group)");
    }
    if (!missingItems.empty()) {
      art::Exception e(art::errors::Configuration);
      e << "SAM metadata information is required -- missing metadata:\n";
      for (auto const s : missingItems) {
        e << s << "\n";
      }
    }
  }

}

art::FileCatalogOptionsHandler::
FileCatalogOptionsHandler(bpo::options_description & desc)
  :
  desc_(desc),
  appFamily_(),
  appVersion_()
{
  desc.add_options()
    ("sam-web-uri", bpo::value<std::string>(), "URI for SAM web service.")
    ("sam-process-id", bpo::value<std::string>(), "SAM process ID.")
    ("sam-application-family", bpo::value<std::string>(&appFamily_), "SAM application family.")
    ("sam-app-family", bpo::value<std::string>(&appFamily_), "SAM application family.")
    ("sam-application-version", bpo::value<std::string>(&appVersion_), "SAM application version.")
    ("sam-app-version", bpo::value<std::string>(&appVersion_), "SAM application version.")
    ("sam-group", bpo::value<std::string>(), "SAM group.")
    ("sam-file-type", bpo::value<std::string>(), "File type for SAM metadata.")
    ("sam-data-tier", bpo::value<std::vector<std::string> >(), "SAM data tier spec-label>:<tier-spec>).")
    ("sam-run-type", bpo::value<std::string>(), "Global run-type for SAM metadata.")
    ("sam-stream-name", bpo::value<std::vector<std::string> >(), "SAM stream name (<module-label>:<stream-name>).")
  ;
}

int
art::FileCatalogOptionsHandler::
doCheckOptions(bpo::variables_map const &)
{
  // Checks can't be done until after post-processing.
  return 0;
}

int
art::FileCatalogOptionsHandler::
doProcessOptions(bpo::variables_map const & vm,
                 fhicl::intermediate_table & raw_config)
{
  auto ciLocation = detail::serviceConfigLocation(raw_config, "CatalogInterface");
  auto ftLocation = detail::serviceConfigLocation(raw_config, "FileTransfer");
  auto fcmdLocation = detail::serviceConfigLocation(raw_config, "FileCatalogMetadata");

  ////////////////////////////////////////////////////////////////////////
  // Load up the configuration with command-line options.
  //
  // sam-web-uri and sam-process-id.
  if (vm.count("sam-web-uri") > 0) {
    raw_config.put(ciLocation + ".webURI",
                   vm["sam-web-uri"].as<std::string>());
  }
  if (vm.count("sam-process-id") > 0) {
    // Sequence.
    raw_config.put("source.fileNames",
                   std::vector<std::string>
                   { vm["sam-process-id"].as<std::string>() });
    // Atom.
    raw_config.put(fcmdLocation + ".processID",
                   vm["sam-process-id"].as<std::string>());
  }
  if (raw_config.exists(ciLocation + ".webURI") !=
      raw_config.exists(fcmdLocation + ".processID")) { // Inconsistent.
    throw Exception(errors::Configuration)
      << "configurations "
      << ciLocation
      << ".webURI (--sam-web-uri) and\n"
      << fcmdLocation
      << ".processID (--sam-process-id) must be specified\n"
      << "together or not at all.\n";
  }
  bool wantSAMweb
  { raw_config.exists(ciLocation + ".webURI") &&
      raw_config.exists("source.fileNames") };
  // Other metadata items.
  if (!appFamily_.empty()) {
    raw_config.put(fcmdLocation + ".applicationFamily",
                   appFamily_);
  }
  if (vm.count("sam-group") > 0) {
    raw_config.put(fcmdLocation + ".group",
                   vm["sam-group"].as<std::string>());
  }
  if (vm.count("sam-run-type") > 0) {
    raw_config.put(fcmdLocation + ".runType",
                   vm["sam-run-type"].as<std::string>());
  }
  if (!appVersion_.empty()) {
    raw_config.put(fcmdLocation + ".applicationVersion",
                   appVersion_);
  }
  if (vm.count("sam-file-type") > 0) {
    raw_config.put(fcmdLocation + ".fileType",
                   vm["sam-file-type"].as<std::string>());
  }
  bool requireMetadata =
    have_outputs(raw_config) &&
    ( wantSAMweb ||
      raw_config.exists(fcmdLocation + ".applicationFamily") ||
      raw_config.exists(fcmdLocation + ".applicationVersion") ||
      raw_config.exists(fcmdLocation + ".fileType") ||
      raw_config.exists(fcmdLocation + ".group")
    );

  if (requireMetadata) {
    fill_tiers_streams(vm, raw_config);
    maybeThrowOnMissingMetadata(raw_config);
  }

  std::string process_name;
  if (raw_config.exists("process_name")) {
    process_name = raw_config.get<std::string>("process_name");
  }
  if (requireMetadata &&
      (process_name.empty())) {
    throw Exception(errors::Configuration)
        << "Non-empty / default process_name required for SAM metadata.\n";
  }
  if (wantSAMweb) {
    raw_config.put(ciLocation + ".service_provider", "IFCatalogInterface");
    raw_config.put(ftLocation + ".service_provider", "IFFileTransfer");
    art::ensureTable(raw_config, detail::serviceConfigLocation(raw_config, "IFDH") );
  }
  return 0;
}
