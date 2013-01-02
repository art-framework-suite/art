#include "art/Framework/Art/artapp.h"

#include "art/Framework/Art/BasicOutputOptionsHandler.h"
#include "art/Framework/Art/BasicSourceOptionsHandler.h"
#include "art/Framework/Art/DebugOptionsHandler.h"
#include "art/Framework/Art/FileCatalogOptionsHandler.h"
#include "art/Framework/Art/OptionsHandlers.h"
#include "art/Framework/Art/run_art.h"
#include "cetlib/filepath_maker.h"

#include "boost/program_options.hpp"

namespace  bpo = boost::program_options;

#include <cstdlib>
#include <iostream>
#include <memory>

int artapp(int argc, char * argv[])
{
  // Configuration file lookup policy.
  if (getenv("FHICL_FILE_PATH") == nullptr) {
    std::cerr
      << "INFO: environment variable FHICL_FILE_PATH was not set. Using \".\"\n";
    setenv("FHICL_FILE_PATH", ".", 0);
  }
 cet::filepath_lookup_after1 lookupPolicy("FHICL_FILE_PATH");
  // Empty options_description.
  bpo::options_description all_desc;
  // Create and store options handlers.
  art::OptionsHandlers handlers;
  handlers.reserve(4); // -ish.
  // Add new handlers here. Do *not* add a BasicOptionsHandler: it will
  // be done for you.
  handlers.emplace_back(new art::BasicSourceOptionsHandler(all_desc));
  handlers.emplace_back(new art::BasicOutputOptionsHandler(all_desc));
  handlers.emplace_back(new art::DebugOptionsHandler(all_desc));
  handlers.emplace_back(new art::FileCatalogOptionsHandler(all_desc));
  return art::run_art(argc, argv, all_desc, lookupPolicy, std::move(handlers));
}

int artapp_string_config(const std::string& config_string)
{
  return art::run_art_string_config(config_string);
}
