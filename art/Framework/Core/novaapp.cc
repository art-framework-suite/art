#include "novaapp.h"
#include "NovaConfigPostProcessor.h"

#include "art/Framework/Core/find_config.h"
#include "art/Framework/Core/run_art.h"
#include "boost/program_options.hpp"
#include "cetlib/exception.h"
#include "cpp0x/memory"
#include "fhiclcpp/parse.h"

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
      ("source,s", bpo::value<std::vector<std::string> >(), "Source data file (multiple OK).")
      ("trace", "Activate tracing.") 
      ("notrace", "Dectivate tracing.") 
      ;

   bpo::positional_options_description pd;
   // A single non-option argument will be taken to be the source data file.
   pd.add("source", -1);

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
   std::string search_path_spec = std::string("./:");
   char const * fhicl_env = getenv("FHICL_FILE_PATH");
   if (fhicl_env != nullptr) search_path_spec += fhicl_env;
   std::string config_filename;
   if (!art::find_config(vm["config"].as<std::string>(),
                         search_path_spec,
                         config_filename)) {
      std::cerr
         << "Specified configuration file "
         << vm["config"].as<std::string>()
         << " cannot be found (search path \""
         << search_path_spec 
         << "\").\n";
      return 7003;
   }
   std::ifstream config_stream(config_filename.c_str());
   if (!config_stream) {
      std::cerr
         << "Specified configuration file "
         << config_filename
         << " cannot be opened for reading.\n";
      return 7004;
   }
   fhicl::intermediate_table raw_config;
   try {
      fhicl::parse_document(config_stream, raw_config);
   }
   catch (cet::exception &e) {
      std::cerr << "Failed to parse the configuration file '"
                << config_filename
                << "' with exception " << e.what()
                << "\n";
      return 7002;
   }
   if ( raw_config.empty() ) {
      std::cerr << "INFO: provided configuration file '"
                << config_filename.c_str()
                << "' is empty: using minimal defaults.\n";
   }

   // Apply our command-line options to the configuration.
   NovaConfigPostProcessor ncpp;
   if (vm.count("trace")) ncpp.trace(true);
   if (vm.count("notrace")) ncpp.trace(false);
   if (vm.count("source"))
      ncpp.sources(vm["source"].as<std::vector<std::string> >());
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
