#include "art/Framework/Art/detail/handle_deprecated_configs.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/intermediate_table.h"

#include <iostream>
#include <sstream>
#include <string>

namespace {
  using fhicl::intermediate_table;
  std::string boundaryFromFileMode(std::string const& fileMode, std::ostringstream& msg)
  {
    bool allowedLegacyParameter {false};
    std::string boundary;
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
  std::string const& param {"services.scheduler.fileMode"};
  if (!raw_config.exists(param)) return;

  std::ostringstream msg;
  msg << "\nThe \"" << param << "\" parameter is deprecated.\n";

  auto const fileMode = raw_config.get<std::string>(param);
  raw_config.erase(param);
  std::string const& boundary = boundaryFromFileMode(fileMode, msg);

  for (auto const& o : raw_config.get<intermediate_table::table_t&>("outputs")) {
    std::string const& module_label = "outputs."+o.first;
    if (raw_config.get<std::string>(module_label+".module_type") != "RootOutput")
      continue;
    raw_config.put(module_label+".fileSwitch.boundary", boundary);
    if (boundary == "InputFile") {
      raw_config.put(module_label+".fileSwitch.force", true);
    }
  }

}
