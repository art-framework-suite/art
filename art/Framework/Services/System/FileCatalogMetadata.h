#ifndef art_Framework_Services_System_FileCatalogMetadata_h
#define art_Framework_Services_System_FileCatalogMetadata_h

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "cetlib/canonical_string.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/fwd.h"

#include <iterator>
#include <string>
#include <vector>

namespace art {
  class FileCatalogMetadata;

  class ActivityRegistry;
}

class art::FileCatalogMetadata {
public:
  typedef std::vector<std::pair<std::string, std::string>> collection_type;
  typedef typename collection_type::value_type value_type;

  FileCatalogMetadata(fhicl::ParameterSet const & ps, ActivityRegistry &);

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
