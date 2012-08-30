#include "art/Framework/Art/BasicSourceOptionsHandler.h"

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
  ("source,s", bpo::value<std::vector<std::string> >(), "Source data file (multiple OK).")
  ("source-list,S", bpo::value<std::string>(), "file containing a list of source files to read, one per line.")
  ("estart,e", bpo::value<unsigned long>(), "Event # of first event to process.")
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
  auto result = processSourceListArg_(vm, source_list);
  if (result != 0) {
    return result;
  }
  // Post-process the config.
  if (source_list.size() > 0) {
    if (!raw_config.exists("source.module_type")) {
      raw_config.put("source.module_type", "RootInput");
    }
    raw_config.put("source.fileNames",
                   source_list);
    if (vm.count("nevts")) {
      raw_config.put("source.maxEvents",
                     vm["nevts"].as<int>());
    }
    if (vm.count("estart")) {
      raw_config.put("source.firstEvent",
                     vm["estart"].as<int>());
    }
    if (vm.count("nskip")) {
      raw_config.put("source.skipEvents",
                     vm["nskip"].as<int>());
    }
  }
  return 0;
}

int
art::BasicSourceOptionsHandler::
processSourceListArg_(bpo::variables_map const & vm,
                      std::vector<std::string> & source_list)
{
  if (vm.count("source-list")) {
    std::ifstream flist(vm["source-list"].as<std::string>().c_str());
    if (!flist) {
      throw Exception(errors::Configuration)
          << "Specified source-list file \""
          << vm["source-list"].as<std::string>()
          << "\" cannot be read.\n";
    }
    while (flist) {
      std::string tmp;
      std::getline(flist, tmp);
      if (tmp.find('#') != std::string::npos) {
        // FIXME: do stuff.
      }
      if (!tmp.empty()) { source_list.push_back(tmp); }
    }
  }
  return 0;
}
