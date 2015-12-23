#include "art/Framework/Art/FileCatalogOptionsHandler.h"

#include "art/Framework/Art/detail/exists_outside_prolog.h"
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
  using art::detail::exists_outside_prolog;

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

  void check_metadata_options(bpo::variables_map const& vm)
  {
    // Mutually exclusive:
    //   --sam-inherit-metadata  --sam-file-type arg
    //   --sam-inherit-file-type --sam-file-type arg
    for( auto const& opt : {"sam-inherit-metadata","sam-inherit-file-type"} ) {
      if (vm.count(opt)+vm.count("sam-file-type") > 1)
        throw art::Exception(art::errors::Configuration)
          << "The options '--" << opt << "' and '--sam-file-type' are mutually exclusive.";
    }

    // Mutually exclusive:
    //   --sam-inherit-metadata --sam-run-type arg
    //   --sam-inherit-run-type --sam-run-type arg
    for( auto const& opt : {"sam-inherit-metadata","sam-inherit-run-type"} ) {
      if (vm.count(opt)+vm.count("sam-run-type") > 1)
        throw art::Exception(art::errors::Configuration)
          << "The options '--" << opt << "' and '--sam-run-type' are mutually exclusive.";
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
    auto const def_tier_it(sep_tiers.find("_default"));
    auto const def_tier((def_tier_it != sep_tiers.end()) ?
                        def_tier_it->second :
                        "");
    auto const def_stream_it(sep_streams.find("_default"));
    auto const def_stream((def_stream_it != sep_streams.end()) ?
                          def_stream_it->second :
                          "");
    for (auto const & output : table) {
      if (!exists_outside_prolog(raw_config, outputs_stem + output.first + ".module_type")) {
        continue; // Not a module parameter set.
      }
      auto const tier_spec_key = outputs_stem + output.first + tier_spec_stem;
      auto const stream_name_key = outputs_stem + output.first + stream_name_stem;
      auto tiers_it(sep_tiers.find(output.first));
      std::string tier;
      if (tiers_it != sep_tiers.end()) {
        tier = tiers_it->second;
      } else if (!exists_outside_prolog(raw_config, tier_spec_key)) {
        tier = def_tier;
      }
      if (!tier.empty()) {
        raw_config.put(tier_spec_key, tier);
      }
      auto streams_it(sep_streams.find(output.first));
      std::string stream;
      if (streams_it != sep_streams.end()) {
        stream = streams_it->second;
      } else if (!exists_outside_prolog(raw_config, stream_name_key)) {
        stream = (!def_stream.empty()) ? def_stream : output.first;
      }
      if (!stream.empty()) {
        raw_config.put(stream_name_key, stream);
      }
      if (raw_config.get<std::string>(outputs_stem + output.first + ".module_type" ) == "RootOutput") {
        if (!(exists_outside_prolog(raw_config, tier_spec_key) &&
              exists_outside_prolog(raw_config, stream_name_key))) {
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
    if (exists_outside_prolog(table, "outputs")) {
      auto const & ev = table.find("outputs");
      if (ev.is_a(fhicl::TABLE) &&
          ! table.get<fhicl::extended_value::table_t const &>("outputs").empty()) {
        result = true;
      }
    }
    return result;
  }

  void maybeThrowOnMissingMetadata(fhicl::intermediate_table const & table) {
    std::string const key_stem { "services.FileCatalogMetadata." };
    std::vector<std::string> missingItems;
    if (!exists_outside_prolog(table, key_stem + "applicationFamily")) {
      missingItems.emplace_back( key_stem + "applicationFamily (--sam-application-family)");
    }
    if (!exists_outside_prolog(table, key_stem + "applicationVersion")) {
      missingItems.emplace_back( key_stem + "applicationVersion (--sam-application-version)");
    }
    if (!exists_outside_prolog(table, key_stem + "group")) {
      missingItems.emplace_back( key_stem + "group (--sam-group)");
    }
    if (!missingItems.empty()) {
      art::Exception e(art::errors::Configuration);
      e << "SAM metadata information is required -- missing metadata:\n";
      for (auto const & s : missingItems) {
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
    ("sam-inherit-metadata","Input file provides the file type and run type.")
    ("sam-inherit-file-type","Input file provides the file type.")
    ("sam-inherit-run-type","Input file provides the run type.")
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
  if (exists_outside_prolog(raw_config, ciLocation + ".webURI") !=
      exists_outside_prolog(raw_config, fcmdLocation + ".processID")) { // Inconsistent.
    throw Exception(errors::Configuration)
      << "configurations "
      << ciLocation
      << ".webURI (--sam-web-uri) and\n"
      << fcmdLocation
      << ".processID (--sam-process-id) must be specified\n"
      << "together or not at all.\n";
  }
  bool wantSAMweb
  { exists_outside_prolog(raw_config, ciLocation + ".webURI") &&
      exists_outside_prolog(raw_config, "source.fileNames") };
  // Other metadata items.
  if (!appFamily_.empty()) {
    raw_config.put(fcmdLocation + ".applicationFamily",
                   appFamily_);
  }
  if (vm.count("sam-group") > 0) {
    raw_config.put(fcmdLocation + ".group",
                   vm["sam-group"].as<std::string>());
  }
  if (!appVersion_.empty()) {
    raw_config.put(fcmdLocation + ".applicationVersion",
                   appVersion_);
  }

  check_metadata_options(vm);

  std::string const mdFromInput {".metadataFromInput"};
  if (vm.count("sam-inherit-metadata") > 0) {
    raw_config.put(fcmdLocation + mdFromInput,
                   std::vector<std::string>{"fileType","runType"});
    raw_config.erase(fcmdLocation + ".fileType");
    raw_config.erase(fcmdLocation + ".runType");
  }
  else {
    std::vector<std::string> md;
    if (vm.count("sam-inherit-file-type") > 0) {
      md.emplace_back("fileType");
      raw_config.erase(fcmdLocation + ".fileType");
    }
    if (vm.count("sam-inherit-run-type") > 0) {
      md.emplace_back("runType");
      raw_config.erase(fcmdLocation + ".runType");
    }
    if (!md.empty()) {
      raw_config.put(fcmdLocation + mdFromInput, md);
    }
  }

  if (vm.count("sam-run-type") > 0) {
    raw_config.put(fcmdLocation + ".runType",
                   vm["sam-run-type"].as<std::string>());
  }
  if (vm.count("sam-file-type") > 0) {
    raw_config.put(fcmdLocation + ".fileType",
                   vm["sam-file-type"].as<std::string>());
  }
  bool requireMetadata =
    have_outputs(raw_config) &&
    ( wantSAMweb ||
      exists_outside_prolog(raw_config, fcmdLocation + ".applicationFamily") ||
      exists_outside_prolog(raw_config, fcmdLocation + ".applicationVersion") ||
      exists_outside_prolog(raw_config, fcmdLocation + ".fileType") ||
      exists_outside_prolog(raw_config, fcmdLocation + ".group")
    );

  if (requireMetadata) {
    fill_tiers_streams(vm, raw_config);
    maybeThrowOnMissingMetadata(raw_config);
  }

  std::string process_name;
  if (exists_outside_prolog(raw_config, "process_name")) {
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
