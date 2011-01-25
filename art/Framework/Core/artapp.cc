#include "art/Framework/Core/artapp.h"

#include "art/Framework/Core/find_config.h"
#include "art/Framework/Core/run_art.h"
#include "boost/program_options.hpp"
#include "fhiclcpp/parse.h"


#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace  bpo=boost::program_options;

int artapp(int argc, char* argv[]) {

   // ------------------
   // use the boost command line option processing library to help out
   // with command line options

   std::ostringstream descstr;

   descstr << argv[0]
           << " <options>";

   bpo::options_description desc(descstr.str());

   desc.add_options()
      ("help,h", "produce help message")
      ("config,c", bpo::value<std::string>(), "configuration file");

   bpo::options_description all_options("All Options");
   all_options.add(desc);

   bpo::variables_map vm;
   try {
      bpo::store(bpo::command_line_parser(argc,argv).options(all_options).run(),vm);
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
   std::string const search_path_spec = "FHICL_FILE_PATH";
   std::string config_filename;
   if (art::find_config(vm["config"].as<std::string>(),
                        search_path_spec,
                        config_filename)) {
      std::cerr
         << "Specified configuration file "
         << vm["config"].as<std::string>()
         << " cannot be found using "
         << search_path_spec 
         << " ("
         << getenv(search_path_spec.c_str())
         << ").\n";
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
