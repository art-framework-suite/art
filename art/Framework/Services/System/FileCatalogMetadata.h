#ifndef art_Framework_Services_System_FileCatalogMetadata_h
#define art_Framework_Services_System_FileCatalogMetadata_h

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "cetlib/canonical_string.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/Atom.h"

#include <iterator>
#include <string>
#include <vector>

namespace art {
  class FileCatalogMetadata;

  class ActivityRegistry;
}

class art::FileCatalogMetadata {
public:
  using collection_type = std::vector<std::pair<std::string, std::string>>;
  using value_type = typename collection_type::value_type;

  static constexpr char const* notPresent = "--optional-parameter--";
  struct Config {
    fhicl::Atom<bool> checkSyntax { fhicl::Name("checkSyntax"), false };
    fhicl::Atom<std::string> applicationFamily  { fhicl::Name("applicationFamily" ), notPresent };
    fhicl::Atom<std::string> applicationVersion { fhicl::Name("applicationVersion"), notPresent };
    fhicl::Atom<std::string> fileType  { fhicl::Name("fileType") , "unknown" };
    fhicl::Atom<std::string> runType   { fhicl::Name("runType")  , notPresent };
    fhicl::Atom<std::string> group     { fhicl::Name("group")    , notPresent };
    fhicl::Atom<std::string> processID { fhicl::Name("processID"), notPresent };
  };

  using Parameters = ServiceTable<Config>;
  FileCatalogMetadata(Parameters const & config, ActivityRegistry &);

  // Add a new value to the metadata store.
  void addMetadata(std::string const & key, std::string const & value);
  // Ensure the value is a canonical string representation.
  void addMetadataString(std::string const & key, std::string const & value);

  void getMetadata(collection_type & coll) const; // Dump stored metadata into the provided container.

  // Ascertain whether JSON syntax checking is desired.
  bool wantCheckSyntax() const { return checkSyntax_; }

private:
  bool const checkSyntax_;
  collection_type md_;
};

inline
void
art::FileCatalogMetadata::
addMetadataString(std::string const & key, std::string const & value)
{
  addMetadata(key, cet::canonical_string(value));
}

inline
void
art::FileCatalogMetadata::
getMetadata(collection_type & coll) const
{
  cet::copy_all(md_, std::back_inserter(coll));
}

DECLARE_ART_SERVICE(art::FileCatalogMetadata, LEGACY)
#endif /* art_Framework_Services_System_FileCatalogMetadata_h */

// Local Variables:
// mode: c++
// End:
