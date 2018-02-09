#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "canvas/Utilities/Exception.h"
IGNORE_FALLTHROUGH_START
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
IGNORE_FALLTHROUGH_END

namespace {
  std::vector<std::string>
  maybeTranslate(std::vector<std::string> names)
  {
    cet::transform_all(names, names.begin(), art::NewToOld{});
    cet::sort_all(names);
    return names;
  }

  bool
  search_translated_all(std::vector<std::string> const& s,
                        std::string const& md)
  {
    return cet::search_all(s, art::NewToOld{}(md));
  }
}

art::FileCatalogMetadata::FileCatalogMetadata(
  art::FileCatalogMetadata::Parameters const& config)
  : checkSyntax_{config().checkSyntax()}
  , mdToInherit_{maybeTranslate(config().metadataFromInput())}
{
  std::string appFam;
  if (config().applicationFamily(appFam)) {
    addMetadataString("applicationFamily", appFam);
  }

  std::string appVer;
  if (config().applicationVersion(appVer)) {
    addMetadataString("applicationVersion", appVer);
  }

  // Always write out fileType -- either by inheriting from the input
  // file or by overriding via the FHiCL parameter "fileType".
  if (!search_translated_all(mdToInherit_, "file_type")) {
    addMetadataString("file_type", config().fileType());
  }

  std::string rt;
  if (!search_translated_all(mdToInherit_, "run_type") &&
      config().runType(rt)) {
    addMetadataString("run_type", rt);
  }

  std::string g;
  if (config().group(g)) {
    addMetadataString("group", g);
  }

  std::string pid;
  if (config().processID(pid)) {
    addMetadataString("process_id", pid);
  }
}

void
art::FileCatalogMetadata::addMetadata(std::string const& key,
                                      std::string const& value)
{
  if (checkSyntax_) {
    // rapidjson claims to be largely re-entrant, according to
    // https://github.com/miloyip/rapidjson/issues/141.  Therefore, we
    // do not worry about locking this part of the function.
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
        << (nSpaces ? std::string(nSpaces, '-') : "") << "^\n";
    }
  }
  std::lock_guard<std::mutex> lock{service_mutex()};
  md_.emplace_back(key, value);
}

// Standard constructor / maker is just fine.
DEFINE_ART_SERVICE(art::FileCatalogMetadata)
