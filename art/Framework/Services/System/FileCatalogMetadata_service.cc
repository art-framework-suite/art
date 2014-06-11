#include "art/Framework/Services/System/FileCatalogMetadata.h"

#include "art/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"

namespace {
  void throw_if_empty(std::string const & par,
                      char const * par_name)
  {
    if (par.empty()) {
      throw art::Exception(art::errors::Configuration, "Missing Required Metadata")
          <<  par_name;
    }
  }
}

art::FileCatalogMetadata::FileCatalogMetadata(fhicl::ParameterSet const & ps,
    ActivityRegistry &)
  :
  md_()
{
  std::string applicationFamily,
    applicationVersion,
    fileType(ps.get<std::string>("fileType", "unknown")),
    runType,
    group,
    processID;
  if (ps.get_if_present("applicationFamily", applicationFamily)) {
    addMetadata("applicationFamily", applicationFamily);
  }
  if (ps.get_if_present("applicationVersion", applicationVersion)) {
    addMetadata("applicationVersion", applicationVersion);
  }
  // Always write out fileType -- may be overridden.
  addMetadata("file_type", fileType);
  if (ps.get_if_present("runType", runType)) {
    addMetadata("run_type", runType);
  }
  if (ps.get_if_present("group", group)) {
    addMetadata("group", group);
  }
  if (ps.get_if_present("processID", processID)) {
    addMetadata("process_id", processID);
  }
}

void
art::FileCatalogMetadata::
addMetadata(std::string const & key, std::string const & value)
{
  md_.emplace_back(key, value);
}

// Standard constructor / maker is just fine.
DEFINE_ART_SERVICE(art::FileCatalogMetadata)
