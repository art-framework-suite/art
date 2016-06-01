#include "art/Framework/Art/detail/handle_deprecated_configs.h"
#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/intermediate_table.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using fhicl::intermediate_table;
using table_t = intermediate_table::table_t;
using sequence_t = intermediate_table::sequence_t;

namespace {
  string boundaryFromFileMode(string const& fileMode, std::ostringstream& msg)
  {
    bool allowedLegacyParameter {false};
    string boundary;
    if (fileMode == "MERGE") {
      allowedLegacyParameter = true;
      boundary = "Unset";
      msg << "The deprecated configuration will be replaced by the following,\n"
          << "which will be added to each of the 'RootOutput' modules\n"
          << "to yield equivalent behavior to the MERGE mode:\n"
          << "  outputs.<module_label>.fileSwitch.boundary: \"Unset\"";
    }
    else if (fileMode == "NOMERGE") {
      allowedLegacyParameter = true;
      boundary = "InputFile";
      msg << "The deprecated configuration will be replaced by the following,\n"
          << "which will be added to each of the 'RootOutput' modules\n"
          << "to yield equivalent behavior to the NOMERGE mode:\n"
          << "  outputs.<module_label>.fileSwitch: {\n"
          << "    boundary: \"InputFile\"\n"
          << "    force: true\n"
          << "  }";
    }
    else if (fileMode == "FULLLUMIMERGE" || fileMode == "FULLMERGE") {
      msg << "The \"" << fileMode << "\" fileMode option is no longer supported.\n"
          << "Please contact artists@fnal.gov for guidance.\n\n";
    }
    else {
      msg << "The \"" << fileMode << "\" fileMode option is not supported.\n"
          << "Please contact artists@fnal.gov for guidance.\n\n";
    }
    return
      allowedLegacyParameter ?
      (std::cerr << msg.str() << "\n\n", boundary) :
      throw art::Exception(art::errors::Configuration) << msg.str();
  }
}

void
art::detail::handle_deprecated_configs(intermediate_table& raw_config)
{
  handle_deprecated_fileMode(raw_config);
  handle_deprecated_SelectEvents(raw_config);
  handle_deprecated_MemoryTracker(raw_config);
}

void
art::detail::handle_deprecated_fileMode(intermediate_table& raw_config)
{
  string const& param {"services.scheduler.fileMode"};
  if (!exists_outside_prolog(raw_config, param)) return;

  std::ostringstream msg;
  msg << "\nThe \"" << param << "\" parameter is deprecated.\n";

  auto const fileMode = raw_config.get<string>(param);
  raw_config.erase(param);
  string const& boundary = boundaryFromFileMode(fileMode, msg);

  for (auto const& o : raw_config.get<table_t&>("outputs")) {
    string const& module_label = "outputs."+o.first;
    if (raw_config.get<string>(module_label+".module_type") != "RootOutput")
      continue;
    raw_config.put(module_label+".fileSwitch.boundary", boundary);
    if (boundary == "InputFile") {
      raw_config.put(module_label+".fileSwitch.force", true);
    }
  }
}

void
art::detail::handle_deprecated_SelectEvents(intermediate_table& raw_config)
{
  auto replace_nested_SelectEvents = [&raw_config](string const& stem) {
    if (!exists_outside_prolog(raw_config, stem)) return;

    for (auto const& mod : raw_config.get<table_t>(stem)) {
      auto const& label = stem+"."+mod.first;
      auto const& se = label+".SelectEvents";
      auto const& sese = se+".SelectEvents";

      bool const sese_exists = exists_outside_prolog(raw_config, sese);
      bool const se_exists = exists_outside_prolog(raw_config, se);

      if (!se_exists)
        continue;

      bool const is_sequence = raw_config.find(se).is_a(fhicl::SEQUENCE);
      if (is_sequence) // Supported configuration
        continue;

      bool const is_table = raw_config.find(se).is_a(fhicl::TABLE);
      if (!is_table) // Configuration error.  Let the validation system take care of it.
        continue;

      std::ostringstream msg;
      sequence_t paths {};
      if (sese_exists) {
        paths = raw_config.get<sequence_t>(sese);
        msg << "\nThe nested \"" << sese << "\" configuration\n"
            << "is deprecated.  It will be replaced with:\n"
            << "   " << se << ": [...]\n";
      }
      else {
        auto const& table = raw_config.get<table_t>(se);
        msg << "\nThe \"" << se << "\" table is deprecated.\n"
            << "It will be replaced with:\n"
            << "   " << se << ": []\n";
      }
      std::cerr << msg.str() << '\n';
      raw_config.erase(se); // Remove SelectEvents table
      raw_config.put(se, paths);
    }
  };

  replace_nested_SelectEvents("outputs");
  replace_nested_SelectEvents("physics.analyzers");
}

void
art::detail::handle_deprecated_MemoryTracker(intermediate_table& raw_config)
{
  string const& stem {"services.MemoryTracker"};
  string const& filename_parameter {stem+".filename"};
  if (!exists_outside_prolog(raw_config, filename_parameter)) return;

  string const& filename = raw_config.get<string>(filename_parameter);
  std::ostringstream msg;
  msg << "\nThe \"" << filename_parameter << "\" parameter is deprecated.  It will be replaced\n"
      << "by the following configuration:\n"
      << "   services.MemoryTracker.db: {\n"
      << "      filename: \"" << filename << "\"\n"
      << "      overwrite: " << std::boolalpha << false << '\n'
      << "   }\n";
  std::cerr << msg.str() << '\n';
  raw_config.erase(filename_parameter);
  string const& db_stem {stem+".db"};
  raw_config.put(db_stem+".filename", filename);
  raw_config.put(db_stem+".overwrite", false);
}
