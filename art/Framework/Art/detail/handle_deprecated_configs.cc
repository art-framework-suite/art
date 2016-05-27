#include "art/Framework/Art/detail/handle_deprecated_configs.h"
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
}

void
art::detail::handle_deprecated_fileMode(intermediate_table& raw_config)
{
  string const& param {"services.scheduler.fileMode"};
  if (!raw_config.exists(param)) return;

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
    if (!raw_config.exists(stem)) return;

    for (auto const& mod : raw_config.get<table_t>(stem)) {
      auto const& label = stem+"."+mod.first;
      auto const& se = label+".SelectEvents.SelectEvents";
      if (!raw_config.exists(se)) continue;

      auto const& pathSequence = raw_config.get<sequence_t>(se);
      std::ostringstream msg;
      msg << "\nThe nested \"" << se << "\" configuration\n"
          << "is deprecated.  It will be replaced with:\n"
          << "   " << label << ".SelectEvents: [...]\n";
      std::cerr << msg.str() << '\n';
      raw_config.erase(label+".SelectEvents.SelectEvents");
      raw_config.put(label+".SelectEvents", pathSequence);
    }
  };

  replace_nested_SelectEvents("outputs");
  replace_nested_SelectEvents("physics.analyzers");
}
