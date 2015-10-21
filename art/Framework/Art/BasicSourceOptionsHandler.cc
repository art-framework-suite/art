#include "art/Framework/Art/BasicSourceOptionsHandler.h"

#include "art/Framework/Art/detail/fillSourceList.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <fstream>
#include <iostream>
#include <string>

art::BasicSourceOptionsHandler::
BasicSourceOptionsHandler(bpo::options_description & desc)
{
  desc.add_options()
    ("source,s", bpo::value<std::vector<std::string> >()->composing(),
     "Source data file (multiple OK); precludes -S.")
    ("source-list,S", bpo::value<std::string>(),
     "file containing a list of source files to read, one per line; "
     "precludes -s.")
    ("estart,e", bpo::value<unsigned long>(),
     "Event # of first event to process.")
    ("nevts,n", bpo::value<int>(), "Number of events to process.")
    ("nskip", bpo::value<unsigned long>(), "Number of events to skip.")
  ;
}

int
art::BasicSourceOptionsHandler::
doCheckOptions(bpo::variables_map const &)
{
  return 0;
}

int
art::BasicSourceOptionsHandler::
doProcessOptions(bpo::variables_map const & vm,
                 fhicl::intermediate_table & raw_config)
{
  std::vector<std::string> source_list;
  if (vm.count("source")) {
    cet::copy_all(vm["source"].as<std::vector<std::string> >(),
                  std::back_inserter(source_list));
  }
  auto have_source_list_file = processSourceListArg_(vm, source_list);
  // Post-process the config.
  if (source_list.size() > 0 || have_source_list_file) {
    // Empty source list file will override non-empty FHiCL spec.
    raw_config.put("source.fileNames",
                   source_list);
  }
  if (vm.count("nevts")) {
    raw_config.put("source.maxEvents",
                   vm["nevts"].as<int>());
  }
  if (vm.count("estart")) {
    raw_config.put("source.firstEvent",
                   vm["estart"].as<unsigned long>());
  }
  if (vm.count("nskip")) {
    raw_config.put("source.skipEvents",
                   vm["nskip"].as<unsigned long>());
  }
  return 0;
}

bool
art::BasicSourceOptionsHandler::
processSourceListArg_(bpo::variables_map const & vm,
                      std::vector<std::string> & source_list)
{
  bool result = !!vm.count("source-list");
  if (result) {
    if (source_list.size()) {
      throw Exception(errors::Configuration)
        << "--source-list (-S) and --source (-s) or non-option arguments are "
        << "incompatible due to ordering ambiguities.\n";
    }
    std::ifstream flist(vm["source-list"].as<std::string>().c_str());
    if (!flist) {
      throw Exception(errors::Configuration)
        << "Specified source-list file \""
        << vm["source-list"].as<std::string>()
        << "\" cannot be read.\n";
    }
    art::detail::fillSourceList(flist, source_list);
  }
  return result;
}
