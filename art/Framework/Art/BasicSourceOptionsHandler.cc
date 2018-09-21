#include "art/Framework/Art/BasicSourceOptionsHandler.h"

#include "art/Framework/Art/detail/event_start.h"
#include "art/Framework/Art/detail/fillSourceList.h"
#include "boost/algorithm/string.hpp"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <fstream>
#include <iostream>
#include <string>

art::BasicSourceOptionsHandler::BasicSourceOptionsHandler(
  bpo::options_description& desc)
{
  bpo::options_description source_options{"Source options"};
  auto options = source_options.add_options();
  add_opt(options,
          "source,s",
          bpo::value<std::vector<std::string>>()->composing(),
          "Source data file (multiple OK); precludes -S.");
  add_opt(options,
          "source-list,S",
          bpo::value<std::string>(),
          "file containing a list of source files to read, one per line; "
          "precludes -s.");
  add_opt(options,
          "estart,e",
          bpo::value<std::string>(),
          "EventID of first event to process (e.g. '1:2:4' starts event "
          "processing at run 1, subrun2, event 4");
  add_opt(
    options, "nevts,n", bpo::value<int>(), "Number of events to process.");
  add_opt(
    options, "nskip", bpo::value<unsigned long>(), "Number of events to skip.");
  desc.add(source_options);
}

int
art::BasicSourceOptionsHandler::doCheckOptions(bpo::variables_map const&)
{
  return 0;
}

int
art::BasicSourceOptionsHandler::doProcessOptions(
  bpo::variables_map const& vm,
  fhicl::intermediate_table& raw_config)
{
  std::vector<std::string> source_list;
  if (vm.count("source")) {
    source_list = vm["source"].as<std::vector<std::string>>();
  }
  auto have_source_list_file = processSourceListArg_(vm, source_list);
  // Post-process the config.
  if (source_list.size() > 0 || have_source_list_file) {
    // Empty source list file will override non-empty FHiCL spec.
    raw_config.put("source.fileNames", source_list);
  }
  if (vm.count("nevts")) {
    raw_config.put("source.maxEvents", vm["nevts"].as<int>());
  }
  if (vm.count("estart")) {
    auto const [run, subRun, event] =
      detail::event_start(vm["estart"].as<std::string>());
    raw_config.put("source.firstRun", run);
    raw_config.put("source.firstSubRun", subRun);
    raw_config.put("source.firstEvent", event);
  }
  if (vm.count("nskip")) {
    raw_config.put("source.skipEvents", vm["nskip"].as<unsigned long>());
  }
  return 0;
}

bool
art::BasicSourceOptionsHandler::processSourceListArg_(
  bpo::variables_map const& vm,
  std::vector<std::string>& source_list)
{
  bool result = !!vm.count("source-list");
  if (result) {
    if (!source_list.empty()) {
      throw Exception(errors::Configuration)
        << "--source-list (-S) and --source (-s) or non-option arguments are "
        << "incompatible due to ordering ambiguities.\n";
    }
    auto const filename = vm["source-list"].as<std::string>();
    std::ifstream flist{filename};
    if (!flist) {
      throw Exception(errors::Configuration)
        << "Specified source-list file \"" << filename
        << "\" cannot be read.\n";
    }
    art::detail::fillSourceList(flist, source_list);
  }
  return result;
}
