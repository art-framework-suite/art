#include "art/Framework/Services/System/FileCatalogMetadata.h"

#include "art/Utilities/Exception.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

namespace {

  std::vector<std::string> maybeTranslate(std::vector<std::string> names)
  {
    std::transform(names.begin(), names.end(),
                   names.begin(), art::NewToOld{});
    cet::sort_all(names);
    return names;
  }

}

art::FileCatalogMetadata::FileCatalogMetadata(art::FileCatalogMetadata::Parameters const & config,
                                              ActivityRegistry &)
  : checkSyntax_{config().checkSyntax()}
  , md_{}
  , imd_{}
  , mdToInherit_{maybeTranslate(config().metadataFromInput())}
{
  std::string appFam;
  if ( config().applicationFamily(appFam) ) addMetadataString("applicationFamily" , appFam);

  std::string appVer;
  if ( config().applicationVersion(appVer) ) addMetadataString("applicationVersion", appVer);

  // Always write out fileType -- either by inheriting from the input
  // file or by overriding via the FHiCL parameter "fileType".
  if ( !inheritedFromInput("file_type") )
    addMetadataString("file_type", config().fileType());

  std::string rt;
  if ( !inheritedFromInput("run_type") && config().runType(rt) ) {
    addMetadataString("run_type", rt);
  }

  std::string g;
  if ( config().group(g) ) addMetadataString("group", g);

  std::string pid;
  if ( config().processID(pid) ) addMetadataString("process_id", pid);

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

bool
art::FileCatalogMetadata::
inheritedFromInput(std::string const& md)
{
  return cet::search_all(mdToInherit_, NewToOld{}(md));
}

// Standard constructor / maker is just fine.
DEFINE_ART_SERVICE(art::FileCatalogMetadata)
