#ifndef art_Framework_Services_System_FileCatalogMetadata_h
#define art_Framework_Services_System_FileCatalogMetadata_h

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "cetlib/canonical_string.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/Sequence.h"

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

  struct Config {
    fhicl::Atom<bool> checkSyntax { fhicl::Name("checkSyntax"), false };
    fhicl::OptionalAtom<std::string> applicationFamily  { fhicl::Name("applicationFamily" ) };
    fhicl::OptionalAtom<std::string> applicationVersion { fhicl::Name("applicationVersion") };
    fhicl::OptionalAtom<std::string> group     { fhicl::Name("group") };
    fhicl::OptionalAtom<std::string> processID { fhicl::Name("processID") };

    fhicl::Sequence<std::string> metadataFromInput {
      fhicl::Name("metadataFromInput"),
      fhicl::Comment("This list specifies the metadata that is inherited\n"
                     "from the input file.  Currently only the run type and\n"
                     "file type metadata can be inherited.  The default list is empty."),
      std::vector<std::string>{}
    };

    bool notInMetadataList(std::string const& name) const
    {
      return !cet::search_all(metadataFromInput(), name);
    }

    fhicl::OptionalAtom<std::string> fileType  { fhicl::Name("fileType"),
        fhicl::Comment("Can specify 'fileType' only if it is not specified\n"
                       "in the 'metadataFromInput' list."),
        [this](){ return notInMetadataList("fileType"); }
    };

    fhicl::OptionalAtom<std::string> runType   { fhicl::Name("runType"),
        fhicl::Comment("Can specify 'runType' only if it is not specified\n"
                       "in the 'metadataFromInput' list."),
        [this](){ return notInMetadataList("runType"); }
    };
  };

  using Parameters = ServiceTable<Config>;
  FileCatalogMetadata(Parameters const & config, ActivityRegistry &);

  // Add a new value to the metadata store.
  void addMetadata(std::string const & key, std::string const & value);
  // Ensure the value is a canonical string representation.
  void addMetadataString(std::string const & key, std::string const & value);

  void getMetadata(collection_type & coll) const; // Dump stored metadata into the provided container.

  // RootInput can set the run-type and file-type parameters
  void setMetadataFromInput(collection_type const& coll);

  // Ascertain whether JSON syntax checking is desired.
  bool wantCheckSyntax() const { return checkSyntax_; }

private:
  bool const checkSyntax_;
  collection_type md_;
  std::vector<std::string> mdFromInput_;
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
setMetadataFromInput(collection_type const& coll)
{
  cet::sort_all(mdFromInput_);
  cet::for_all(coll, [this](auto const& pr) {
      if ( cet::search_all(mdFromInput_, pr.first) )
        this->addMetadataString(pr.first, pr.second);
    } );
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
