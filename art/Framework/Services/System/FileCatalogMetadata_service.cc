#include "art/Framework/Services/System/FileCatalogMetadata.h"

#include "art/Utilities/Exception.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

art::FileCatalogMetadata::FileCatalogMetadata(art::FileCatalogMetadata::Parameters const & config,
                                              ActivityRegistry &)
  : checkSyntax_( config().checkSyntax() )
  , md_()
{
  std::string const applicationFamily  = config().applicationFamily();
  std::string const applicationVersion = config().applicationVersion();
  std::string const fileType  = config().fileType();
  std::string const runType   = config().runType();
  std::string const group     = config().group();
  std::string const processID = config().processID();

  if (applicationFamily  != notPresent) addMetadataString("applicationFamily" , applicationFamily );
  if (applicationVersion != notPresent) addMetadataString("applicationVersion", applicationVersion);

  // Always write out fileType -- may be overridden.
  addMetadataString("file_type", fileType);

  if (runType   != notPresent) addMetadataString("run_type"  , runType  );
  if (group     != notPresent) addMetadataString("group"     , group    );
  if (processID != notPresent) addMetadataString("process_id", processID);
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
