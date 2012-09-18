#ifndef art_Framework_Services_System_FileCatalogMetadata_h
#define art_Framework_Services_System_FileCatalogMetadata_h

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

  FileCatalogMetadata(fhicl::ParameterSet const &, ActivityRegistry &);

  // Add a new value to the metadata store.
  void addMetadata(std::string const & key, std::string const & value);
  void getMetadata(collection_type & coll) const; // Dump stored metadata into the provided container.

private:
  collection_type md_;
};

inline
void
art::FileCatalogMetadata::
getMetadata(collection_type & coll) const
{
  cet::copy_all(md_, std::back_inserter(coll));
}

#endif /* art_Framework_Services_System_FileCatalogMetadata_h */

// Local Variables:
// mode: c++
// End:
