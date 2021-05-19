#include "art/Framework/Services/System/FileCatalogMetadata.h"

#include "canvas/Utilities/Exception.h"
#include "cetlib/canonical_string.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/Sequence.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include <string>
#include <vector>

using namespace std;

namespace art {

  namespace {
    detail::OldToNew const oldToNew{};
    vector<string>
    maybeTranslate(vector<string> names)
    {
      cet::transform_all(names, names.begin(), oldToNew);
      cet::sort_all(names);
      return names;
    }

    bool
    search_translated_all(vector<string> const& s, string const& md)
    {
      return cet::search_all(s, oldToNew(md));
    }
  } // unnamed namespace

  FileCatalogMetadata::FileCatalogMetadata(
    FileCatalogMetadata::Parameters const& config)
    : checkSyntax_{config().checkSyntax()}
    , mdToInherit_{maybeTranslate(config().metadataFromInput())}
  {
    string appFam;
    if (config().applicationFamily(appFam)) {
      addMetadataString("application.family", appFam);
    }
    string appVer;
    if (config().applicationVersion(appVer)) {
      addMetadataString("application.version", appVer);
    }

    // Always write out fileType -- either by inheriting from the input
    // file or by overriding via the FHiCL parameter "fileType".
    if (!search_translated_all(mdToInherit_, "file_type")) {
      addMetadataString("file_type", config().fileType());
    }
    string rt;
    if (!search_translated_all(mdToInherit_, "run_type") &&
        config().runType(rt)) {
      addMetadataString("art.run_type", rt);
    }
    string grp;
    if (config().group(grp)) {
      addMetadataString("group", grp);
    }
    string pid;
    if (config().processID(pid)) {
      addMetadataString("process_id", pid);
    }
  }

  bool
  FileCatalogMetadata::wantCheckSyntax() const
  {
    return checkSyntax_;
  }

  void
  FileCatalogMetadata::addMetadataString(string const& key, string const& value)
  {
    addMetadata(key, cet::canonical_string(value));
  }

  void
  FileCatalogMetadata::addMetadata(string const& key, string const& value)
  {
    std::lock_guard sentry{mutex_};
    if (checkSyntax_) {
      // rapidjson claims to be largely re-entrant, according to
      // https://github.com/miloyip/rapidjson/issues/141.  Therefore, we
      // do not worry about locking this part of the function.
      rapidjson::Document d;
      string checkString("{ ");
      checkString += cet::canonical_string(key);
      checkString += " : ";
      checkString += value;
      checkString += " }";
      if (d.Parse(checkString.c_str()).HasParseError()) {
        auto const nSpaces = d.GetErrorOffset();
        throw Exception(errors::DataCorruption)
          << "FileCatalogMetadata::addMetadata() JSON syntax error:\n"
          << rapidjson::GetParseError_En(d.GetParseError())
          << " Faulty key/value clause:\n"
          << checkString << "\n"
          << (nSpaces ? string(nSpaces, '-') : "") << "^\n";
      }
    }
    md_.emplace_back(key, value);
  }

  void
  FileCatalogMetadata::setMetadataFromInput(collection_type const& mdFromInput)
  {
    std::lock_guard sentry{mutex_};
    if (mdToInherit_.empty()) {
      return;
    }
    if (!imd_) {
      imd_ = make_unique<InheritedMetadata>(mdToInherit_, mdFromInput);
    } else {
      imd_->check_values(mdFromInput);
    }
    for (auto const& pr : imd_->entries()) {
      addMetadataString(oldToNew(pr.first), pr.second);
    }
  }

  void
  FileCatalogMetadata::getMetadata(collection_type& coll) const
  {
    std::lock_guard sentry{mutex_};
    cet::copy_all(md_, back_inserter(coll));
  }

} // namespace art
