#include "art/Framework/Art/BasicOutputOptionsHandler.h"

#include "art/Utilities/ensureTable.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <iostream>
#include <string>

namespace {
  using table_t = fhicl::extended_value::table_t;
  using sequence_t = fhicl::extended_value::sequence_t;
}

art::BasicOutputOptionsHandler::
BasicOutputOptionsHandler(bpo::options_description & desc)
{
  desc.add_options()
  ("TFileName,T", bpo::value<std::string>(), "File name for TFileService.")
  ("output,o", bpo::value<std::string>(), "Event output stream file.")
  ;
}

int
art::BasicOutputOptionsHandler::
doCheckOptions(bpo::variables_map const &)
{
  return 0;
}

int
art::BasicOutputOptionsHandler::
doProcessOptions(bpo::variables_map const & vm,
                 fhicl::intermediate_table & raw_config)
{
  // TFileService output.
  if (vm.count("TFileName") == 1) {
    std::string tFileName(vm["TFileName"].as<std::string>());
    if (tFileName.empty() &&
        raw_config.exists("services.TFileService") &&
        raw_config.get<table_t const &>("services.TFileService").empty()) {
      tFileName = "hist.root";
    }
    if (!tFileName.empty()) {
      raw_config.put("services.TFileService.fileName",
                     tFileName);
    }
  }
  // File output.
  if (vm.count("output") == 1) {
    std::string output(vm["output"].as<std::string>());
    if (!output.empty()) {
      bool new_path_entry(false);
      ensureTable(raw_config, "outputs");
      auto & outputs_table(raw_config.get<table_t &>("outputs"));
      if (outputs_table.empty()) {
        new_path_entry = true;
        raw_config.put("outputs.out.module_type", "RootOutput");
      }
      else if (outputs_table.size() > 1) {
        throw cet::exception("BAD_OUTPUT_CONFIG")
            << "Output configuration is ambiguous: configuration has "
            << "multiple output modules. Cannot decide where to add "
            << "specified output filename "
            << output
            << ".";
      }
      else {
        // Empty.
      }
      std::string out_table_name(outputs_table.begin()->first);
      assert(!out_table_name.empty());
      std::string out_table_path("outputs");
      out_table_path += "." + out_table_name;
      raw_config.put(out_table_path + ".fileName", output);
      if (new_path_entry) {
        // If we created a new output module config, we need to make a
        // path for it and add it to end paths. We will *not* detect the
        // case where an *existing* output module config is not
        // referenced in a path.
        ensureTable(raw_config, "physics");
        auto & physics_table = raw_config.get<table_t &>("physics");
        // Find an unique name for the end_path into which we'll insert
        // our new module.
        std::string end_path = "injected_end_path_";
        size_t n = physics_table.size() + 2;
        for (size_t i = 1; i < n; ++i) {
          std::ostringstream os;
          os << end_path << i;
          if (physics_table.find(os.str()) == physics_table.end()) {
            end_path = os.str();
            break;
          }
        }
        // Make our end_path with the output module label in it.
        raw_config.put(std::string("physics.") + end_path + "[0]",
                       out_table_name);
        // Add it to the end_paths list.
        size_t index =
          raw_config.exists("physics.end_paths") ?
          raw_config.get<sequence_t &>("physics.end_paths").size() :
          0;
        raw_config.put(std::string("physics.end_paths[") +
                       std::to_string(index) + "]", end_path);
      }
    }
  }
  return 0;
}
