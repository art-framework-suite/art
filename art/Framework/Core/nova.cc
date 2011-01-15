#include "boost/program_options.hpp"
#include "fhiclcpp/parse.h"

#include "art/Framework/Core/run_art.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace  bpo=boost::program_options;

int main(int argc, char* argv[]) {

   // ------------------
   // use the boost command line option processing library to help out
   // with command line options

   std::ostringstream descstr;

   descstr << argv[0]
           << " <options> [config-file]";

   bpo::options_description desc(descstr.str());

   desc.add_options()
      ("help,h", "produce help message")
      ("config,c", bpo::value<std::string>(), "Configuration file.")
      ("source,s", bpo::value<std::string>(), "Source data file (one only).")
      ("TFileName,T", bpo::value<std::string>(), "File name for TFileService.")
      ("output,o", bpo::value<std::string>(), "Event output stream file.")
      ("nevts,n", bpo::value<int>(), "Number of events to process.")
      ("estart,e", bpo::value<int>(), "Event # of first event to process.")
      ("nskip", bpo::value<int>(), "Number of events to skip.")
      ;

   bpo::positional_options_description pd;
   pd.add("config", 1);

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
   std::string config_filename = vm["config"].as<std::string>();
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

   return art::run_art(raw_config);
}
