#include "art/Framework/Art/BasicSourceOptionsHandler.h"

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
          "Event # of first event to process.");
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

namespace {
  std::string const context{"An error was encountered while processing the "
                            "-e|--estart program option.\n"};

  template <typename T>
  std::enable_if_t<std::is_arithmetic_v<T>, T>
  safe_conversion(std::string const& str_num)
  {
    unsigned long long num{-1ull};
    try {
      num = std::stoull(str_num);
    }
    catch (std::exception const& e) {
      throw art::Exception{art::errors::Configuration, context}
        << "The following std::exception was thrown while attempting to "
           "convert '"
        << str_num << "':\n"
        << e.what() << "\n";
    }
    if (num > std::numeric_limits<T>::max()) {
      throw art::Exception{art::errors::Configuration, context}
        << "The value " << str_num << " is not representable in an EventID.\n"
        << "Please choose a value in the half-closed interval:\n"
        << "  [" << std::numeric_limits<T>::min() << ", "
        << std::numeric_limits<T>::max() << ")\n";
    }
    return static_cast<T>(num);
  }

  auto
  starting_event(std::string const& event_spec)
  {
    std::vector<std::string> parts;
    boost::split(parts, event_spec, boost::is_any_of(":"));
    if (parts.size() == 1ull) {
      std::cerr
        << "\nSpecifying an event number of " << parts[0]
        << " is now deprecated when using\n"
        << "the -e|--estart program option.  Please explicitly specify the\n"
        << "run and subrun numbers (e.g.):\n"
        << "   -e \"1:0:" << parts[0] << "\"\n\n";
      auto const e = safe_conversion<art::EventNumber_t>(parts[0]);
      return std::make_tuple(art::IDNumber<art::Level::Run>::first(),
                             art::IDNumber<art::Level::SubRun>::first(),
                             e);
    } else if (parts.size() == 3ull) {
      auto const r = safe_conversion<art::RunNumber_t>(parts[0]);
      auto const sr = safe_conversion<art::SubRunNumber_t>(parts[1]);
      auto const e = safe_conversion<art::EventNumber_t>(parts[2]);
      return std::make_tuple(r, sr, e);
    }
    throw art::Exception{art::errors::Configuration, context}
      << "It is illegal to specify an event with " << parts.size()
      << " colons.\n"
      << "Please use the format \"run:subrun:event\".\n";
  }
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
      starting_event(vm["estart"].as<std::string>());
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
