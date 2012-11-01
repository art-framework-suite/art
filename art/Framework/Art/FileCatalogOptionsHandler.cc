#include "art/Framework/Art/FileCatalogOptionsHandler.h"

#include "art/Utilities/Exception.h"
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
    auto const & table = raw_config.get<table_t const &>("outputs");
    if (table.size() == 0) {
      return;
    }
    std::string const outputs_stem("outputs.");
    std::string const tier_spec_stem(".dataTier");
    std::string const stream_name_stem(".streamName");
    std::vector<std::string> data_tiers(vm["sam-data-tier"].as<std::vector<std::string> >());
    std::vector<std::string>
      stream_names((vm.count("sam-stream-name") == 1) ?
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
      if (!raw_config.exists(outputs_stem + output.first + tier_spec_stem)) {
        throw art::Exception(art::errors::Configuration)
          << "Unspecified data tier for output \""
          << output.first
          << "\"\n";
      }
      auto streams_it(sep_streams.find(output.first));
      std::string stream((streams_it != sep_streams.end()) ?
                         streams_it->second :
                         def_stream);
      raw_config.put(outputs_stem + output.first + stream_name_stem,
                     (!stream.empty()) ?
                     stream :
                     output.first);
    }
  }
}

art::FileCatalogOptionsHandler::
FileCatalogOptionsHandler(bpo::options_description & desc)
  :
  desc_(desc),
  requireMetadata_(false),
  wantSAMweb_(false),
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
  ("sam-file-type", bpo::value<std::string>(), "File type for SAM metadata.")
  ("sam-data-tier", bpo::value<std::vector<std::string> >(), "SAM data tier spec (<module-label>:<tier-spec>).")
  ("sam-stream-name", bpo::value<std::vector<std::string> >(), "SAM stream name (<module-label>:<stream-name>).")
  ;
}

int
art::FileCatalogOptionsHandler::
doCheckOptions(bpo::variables_map const & vm)
{
  requireMetadata_ =
    vm.count("sam-application-family") > 0 ||
    vm.count("sam-app-family") > 0 ||
    vm.count("sam-application-version") > 0 ||
    vm.count("sam-app-version") > 0 ||
    vm.count("sam-file-type") > 0 ||
    vm.count("sam-data-tier") > 0 ||
    vm.count("sam-stream-name") > 0;
  if (vm.count("sam-web-uri") || vm.count("sam-process-id")) {
    wantSAMweb_ = true;
    if (vm.count("sam-web-uri") == 1 && vm.count("sam-process-id") == 1) {
      requireMetadata_ = true;
    }
    else {
      throw Exception(errors::Configuration)
          << "--sam-web-uri and --sam-process-id must be used together or "
          << "not at all.\n";
    }
  }
  return 0;
}

int
art::FileCatalogOptionsHandler::
doProcessOptions(bpo::variables_map const & vm,
                 fhicl::intermediate_table & raw_config)
{
  // Couldn't do this check earlier.
  if (requireMetadata_ && !raw_config.exists("outputs")) {
    // Don't need metadata after all.
    requireMetadata_ = false;
  }
  if (requireMetadata_ &&
      (appFamily_.empty() ||
       appVersion_.empty() ||
       vm.count("sam-file-type") == 0 ||
       vm.count("sam-data-tier") == 0)) {
    throw Exception(errors::Configuration)
        << "SAM metadata information is required -- missing metadata:"
        << (appFamily_.empty() ? "\n--sam-application-family" : "")
        << (appVersion_.empty() ? "\n--sam-application-version" : "")
        << ((vm.count("sam-file-type") == 0) ? "\n--sam-file-type" : "")
        << ((vm.count("sam-data-tier") == 0) ? "\n--sam-data-tier" : "")
        << "\n";
  }
  std::string process_name;
  if (raw_config.exists("process_name")) {
    process_name = raw_config.get<std::string>("process_name");
  }
  if (requireMetadata_ &&
      (process_name.empty())) {
    throw Exception(errors::Configuration)
        << "Non-empty / default process_name required for SAM metadata.\n";
  }
  if (wantSAMweb_) {
    raw_config.put("services.user.CatalogInterface.service_provider", "IFCatalogInterface");
    raw_config.put("services.user.CatalogInterface.webURI", vm["sam-web-uri"].as<std::string>());
    raw_config.put("services.user.FileTransfer.service_provider", "IFFileTransfer");
    raw_config.putEmptyTable("services.user.IFDH");
    raw_config.put("source.fileNames", std::vector<std::string> { vm["sam-process-id"].as<std::string>() });
  }
  if (requireMetadata_) {
    raw_config.put("services.FileCatalogMetadata.applicationFamily",
                   vm["sam-application-family"].as<std::string>());
    raw_config.put("services.FileCatalogMetadata.applicationVersion",
                   vm["sam-application-version"].as<std::string>());
    raw_config.put("services.FileCatalogMetadata.fileType.",
                   vm["sam-file-type"].as<std::string>());
    fill_tiers_streams(vm, raw_config);
  }
  return 0;
}
