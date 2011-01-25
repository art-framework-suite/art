#include "novaapp.h"

#include "art/Framework/Core/run_art.h"
#include "cetlib/search_path.h"
#include "cetlib/exception.h"
#include "fhiclcpp/parse.h"

#include "NovaConfigPostProcessor.h"

#include "boost/program_options.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace  bpo=boost::program_options;

int novaapp(int argc, char* argv[]) {

   // ------------------
   // use the boost command line option processing library to help out
   // with command line options

   std::ostringstream descstr;

   descstr << argv[0]
           << " <options> [config-file]";

   bpo::options_description desc(descstr.str());

   desc.add_options()
      ("TFileName,T", bpo::value<std::string>(), "File name for TFileService.")
      ("config,c", bpo::value<std::string>(), "Configuration file.")
      ("estart,e", bpo::value<unsigned long>(), "Event # of first event to process.")
      ("help,h", "produce help message")
      ("nevts,n", bpo::value<int>(), "Number of events to process.")
      ("nskip", bpo::value<unsigned long>(), "Number of events to skip.")
      ("output,o", bpo::value<std::string>(), "Event output stream file.")
      ("source,s", bpo::value<std::string>(), "Source data file (one only).")
      ("trace", "Activate tracing.") 
      ("notrace", "Dectivate tracing.") 
      ;

   bpo::positional_options_description pd;
   // A single non-option argument will be taken to be the source data file.
   pd.add("source", 1);

   bpo::variables_map vm;
   try {
      bpo::store(bpo::command_line_parser(argc,argv).options(desc).positional(pd).run(),vm);
      bpo::notify(vm);
   }
   catch(bpo::error const& e) {
      std::cerr << "Exception from command line processing in " << argv[0]
                << ": " << e.what() << "\n";
      return 7000;
   }

   if (vm.count("help")) {
      std::cout << desc <<std::endl;
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

   //
   // Get the parameter set by parsing the configuration file.
   //
   fhicl::intermediate_table raw_config;
   std::string config_filename;
   try {
   cet::search_path sp("FHICL_FILE_PATH");
   if (!sp.find_file(vm["config"].as<std::string>(), config_filename)) {
      std::cerr
         << "Specified configuration file "
         << vm["config"].as<std::string>()
         << " cannot be found using FHICL_SEARCH_PATH ("
         << getenv("FHICL_SEARCH_PATH")
         << ").\n";
      return 7004;
   }
   }
   catch (cet::exception const &e) {
      if (e.root_cause() == "getenv") {
         // Assume file is findable as specified.
         config_filename = vm["config"].as<std::string>();
      } else {
         throw;
      }
   }
   std::ifstream config_stream(config_filename.c_str());
   if (!config_stream) {
      std::cerr
         << "Specified configuration file "
         << config_filename
         << " cannot be opened for reading.\n";
      return 7004;
   }
   if (!fhicl::parse_document(config_stream, raw_config)) {
      std::cerr << "Failed to parse the configuration file '"
                << config_filename
                << "'\n";
      return 7002;
   } else if ( raw_config.empty() ) {
      std::cerr << "INFO: provided configuration file '"
                << config_filename.c_str()
                << "' is empty: using minimal defaults.\n";
   }

   // Apply our command-line options to the configuration.
   NovaConfigPostProcessor ncpp;
   if (vm.count("trace")) ncpp.trace(true);
   if (vm.count("notrace")) ncpp.trace(false);
   if (vm.count("source"))
      ncpp.source(vm["source"].as<std::string>());
   if (vm.count("TFileName"))
      ncpp.tFileName(vm["TFileName"].as<std::string>());
   if (vm.count("output"))
      ncpp.output(vm["output"].as<std::string>());
   if (vm.count("nevts"))
      ncpp.nevts(vm["nevts"].as<int>());
   if (vm.count("estart"))
      ncpp.startEvt(vm["estart"].as<unsigned long>());
   if (vm.count("nskip"))
      ncpp.skipEvts(vm["nskip"].as<unsigned long>());
   ncpp.apply(raw_config);

   return art::run_art(raw_config);
}
