#include "TError.h"
#include "art/Framework/Core/EventProcessor.h"
#include "art/Framework/Core/IntermediateTablePostProcessor.h"
#include "art/Framework/Core/RootDictionaryManager.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Framework/Services/Registry/ServiceWrapper.h"
#include "art/Persistency/Common/InitRootHandlers.h"
#include "art/Utilities/ExceptionMessages.h"
#include "art/Utilities/RootHandlers.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "boost/program_options.hpp"
#include "boost/shared_ptr.hpp"
#include "cetlib/exception.h"
#include "fhiclcpp/parse.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "art/Framework/Core/run_art.h"

#include <cstring>
#include <exception>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace  bpo=boost::program_options;
using std::string;
using std::ostringstream;
using std::ifstream;

extern "C" { int artapp(int argc, char *argv[]); }

int artapp(int argc, char* argv[]) {

   // ------------------
   // use the boost command line option processing library to help out
   // with command line options

   ostringstream descstr;

   descstr << argv[0]
           << " <options>";

   bpo::options_description desc(descstr.str());

   desc.add_options()
      ("help,h", "produce help message")
      ("config,c", bpo::value<string>(), "configuration file");

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
   fhicl::intermediate_table raw_config;
   string config_filename = vm["config"].as<string>();
   ifstream config_stream(config_filename.c_str());
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
