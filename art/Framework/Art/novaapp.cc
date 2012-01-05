#include "art/Framework/Art/novaapp.h"

#include "art/Framework/Art/RichConfigPostProcessor.h"
#include "art/Framework/Art/find_config.h"
#include "art/Framework/Art/run_art.h"
#include "art/Utilities/FirstAbsoluteOrLookupWithDotPolicy.h"
#include "boost/program_options.hpp"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "fhiclcpp/parse.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace  bpo = boost::program_options;

int novaapp(int argc, char * argv[])
{
  // ------------------
  // use the boost command line option processing library to help out
  // with command line options
  std::ostringstream descstr;
  descstr << argv[0]
          << " <-c <config-file>> <other-options> [<source-file>]+";
  bpo::options_description desc(descstr.str());
  desc.add_options()
  ("TFileName,T", bpo::value<std::string>(), "File name for TFileService.")
  ("config,c", bpo::value<std::string>(), "Configuration file.")
  ("estart,e", bpo::value<unsigned long>(), "Event # of first event to process.")
  ("help,h", "produce help message")
  ("nevts,n", bpo::value<int>(), "Number of events to process.")
  ("nskip", bpo::value<unsigned long>(), "Number of events to skip.")
  ("output,o", bpo::value<std::string>(), "Event output stream file.")
  ("source,s", bpo::value<std::vector<std::string> >(), "Source data file (multiple OK).")
  ("source-list,S", bpo::value<std::string>(), "file containing a list of source files to read, one per line.")
  ("trace", "Activate tracing.")
  ("notrace", "Deactivate tracing.")
  ("memcheck", "Activate monitoring of memory use.")
  ("nomemcheck", "Deactivate monitoring of memory use.")
  ("rethrow-default", "all exceptions default to rethrow.")
  ("rethrow-all", "all exceptions overridden to rethrow (cf rethrow-default).")
  ;
  bpo::positional_options_description pd;
  // A single non-option argument will be taken to be the source data file.
  pd.add("source", -1);
  bpo::variables_map vm;
  try {
    bpo::store(bpo::command_line_parser(argc, argv).options(desc).positional(pd).run(), vm);
    bpo::notify(vm);
  }
  catch (bpo::error const & e) {
    std::cerr << "Exception from command line processing in " << argv[0]
              << ": " << e.what() << "\n";
    return 7000;
  }
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }
  if (!vm.count("config")) {
    std::cerr << "Exception from command line processing in " << argv[0]
              << ": no configuration file given.\n"
              << "For usage and an options list, please do '"
              << argv[0] <<  " --help"
              << "'.\n";
    return 7001;
  }
  std::vector<std::string> source_list;
  if (vm.count("source")) {
    cet::copy_all(vm["source"].as<std::vector<std::string> >(),
                  std::back_inserter(source_list));
  }
  if (vm.count("source-list")) {
    std::ifstream flist(vm["source-list"].as<std::string>().c_str());
    if (!flist) {
      std::cerr << "Exception in command line processing in " << argv[0]
                << ": specified source-list file \""
                << vm["source-list"].as<std::string>()
                << "\" cannot be read.\n";
      return 7001;
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
  //
  // Get the parameter set by parsing the configuration file.
  //
  fhicl::intermediate_table raw_config;
  char const * fhicl_env = getenv("FHICL_FILE_PATH");
  std::string search_path;
  if (fhicl_env == nullptr) {
    std::cerr
        << "Expected environment variable FHICL_FILE_PATH is "
        << "missing or empty: using \".\"\n";
    search_path = ".:";
  }
  else {
    search_path = std::string(fhicl_env) + ":";
  }
  art::FirstAbsoluteOrLookupWithDotPolicy lookupPolicy(search_path);
  try {
    fhicl::parse_document(vm["config"].as<std::string>(), lookupPolicy, raw_config);
  }
  catch (cet::exception & e) {
    std::cerr << "Failed to parse the configuration file '"
              << vm["config"].as<std::string>()
              << "' with exception\n" << e.what()
              << "\n";
    return 7002;
  }
  if (raw_config.empty()) {
    std::cerr << "INFO: provided configuration file '"
              << vm["config"].as<std::string>()
              << "' is empty: using minimal defaults.\n";
  }
  // Apply our command-line options to the configuration.
  RichConfigPostProcessor rcpp;
  if (vm.count("trace")) {
    rcpp.trace(true);
  }
  else if (vm.count("notrace")) {
    rcpp.trace(false);
  }
  if (vm.count("memcheck")) {
    rcpp.memcheck(true);
  }
  else if (vm.count("nomemcheck")) {
    rcpp.memcheck(false);
  }
  if (!source_list.empty()) { rcpp.sources(source_list); }
  if (vm.count("TFileName"))
  { rcpp.tFileName(vm["TFileName"].as<std::string>()); }
  if (vm.count("output"))
  { rcpp.output(vm["output"].as<std::string>()); }
  if (vm.count("nevts"))
  { rcpp.nevts(vm["nevts"].as<int>()); }
  if (vm.count("estart"))
  { rcpp.startEvt(vm["estart"].as<unsigned long>()); }
  if (vm.count("nskip"))
  { rcpp.skipEvts(vm["nskip"].as<unsigned long>()); }
  rcpp.apply(raw_config);
  return art::run_art(raw_config, vm);
}
