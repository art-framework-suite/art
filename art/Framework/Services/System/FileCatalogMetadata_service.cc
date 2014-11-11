#include "art/Framework/Services/System/FileCatalogMetadata.h"

#include "art/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

art::FileCatalogMetadata::FileCatalogMetadata(fhicl::ParameterSet const & ps,
    ActivityRegistry &)
  :
  checkSyntax_(ps.get<bool>("checkSyntax", false)),
  md_()
{
  std::string applicationFamily,
    applicationVersion,
    fileType(ps.get<std::string>("fileType", "unknown")),
    runType,
    group,
    processID;
  if (ps.get_if_present("applicationFamily", applicationFamily)) {
    addMetadataString("applicationFamily", applicationFamily);
  }
  if (ps.get_if_present("applicationVersion", applicationVersion)) {
    addMetadataString("applicationVersion", applicationVersion);
  }
  // Always write out fileType -- may be overridden.
  addMetadataString("file_type", fileType);
  if (ps.get_if_present("runType", runType)) {
    addMetadataString("run_type", runType);
  }
  if (ps.get_if_present("group", group)) {
    addMetadataString("group", group);
  }
  if (ps.get_if_present("processID", processID)) {
    addMetadataString("process_id", processID);
  }
}

void
art::FileCatalogMetadata::
addMetadata(std::string const & key, std::string const & value)
{
  if (checkSyntax_) {
    rapidjson::Document d;
    std::string checkString("{ ");
    checkString += cet::canonical_string(key);
    checkString += " : ";
    checkString += value;
    checkString += " }";
    if (d.Parse(checkString.c_str()).HasParseError()) {
      auto const nSpaces = d.GetErrorOffset();
      throw Exception(errors::DataCorruption)
        << "art::FileCatalogMetadata::addMetadata() JSON syntax error:\n"
        << rapidjson::GetParseError_En(d.GetParseError())
        << " Faulty key/value clause:\n"
        << checkString << "\n"
        << (nSpaces ? std::string(nSpaces, '-') : "")
        << "^\n";
    }
  }
  md_.emplace_back(key, value);
}

// Standard constructor / maker is just fine.
DEFINE_ART_SERVICE(art::FileCatalogMetadata)
