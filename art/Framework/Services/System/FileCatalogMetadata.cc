#include "art/Framework/Services/System/FileCatalogMetadata.h"

#include "boost/json.hpp"
#include "canvas/Utilities/Exception.h"
#include "cetlib/canonical_string.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/Sequence.h"

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
    if (auto appFam = config().applicationFamily()) {
      addMetadataString("application.family", *appFam);
    }
    if (auto appVer = config().applicationVersion()) {
      addMetadataString("application.version", *appVer);
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
    if (auto grp = config().group()) {
      addMetadataString("group", *grp);
    }
    if (auto pid = config().processID()) {
      addMetadataString("process_id", *pid);
    }
  }

  bool
  FileCatalogMetadata::wantCheckSyntax() const noexcept
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
    if (checkSyntax_) {
      string checkString("{ ");
      checkString += cet::canonical_string(key) + " : " + value + " }";
      boost::json::error_code ec;
      boost::json::parser p;
      auto const n_parsed_chars = p.write_some(checkString, ec);
      if (ec) {
        throw Exception(errors::DataCorruption)
          << "FileCatalogMetadata::addMetadata() JSON " << ec.message() << ":\n"
          << "Faulty key/value clause:\n"
          << checkString << '\n'
          << (n_parsed_chars ? string(n_parsed_chars, '-') : "") << "^\n";
      }
    }
    std::lock_guard sentry{mutex_};
    md_.emplace_back(key, value);
  }

  void
  FileCatalogMetadata::setMetadataFromInput(collection_type const& mdFromInput)
  {
    if (mdToInherit_.empty()) {
      return;
    }

    std::lock_guard sentry{mutex_};
    if (!imd_) {
      imd_ = make_unique<InheritedMetadata>(mdToInherit_, mdFromInput);
    } else {
      imd_->check_values(mdFromInput);
    }
    for (auto const& [key, value] : imd_->entries()) {
      addMetadataString(oldToNew(key), value);
    }
  }

  void
  FileCatalogMetadata::getMetadata(collection_type& coll) const
  {
    std::lock_guard sentry{mutex_};
    cet::copy_all(md_, back_inserter(coll));
  }

} // namespace art
