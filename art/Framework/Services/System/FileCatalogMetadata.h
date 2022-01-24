#ifndef art_Framework_Services_System_FileCatalogMetadata_h
#define art_Framework_Services_System_FileCatalogMetadata_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceDeclarationMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Framework/Services/System/detail/SAMMetadataTranslators.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/Sequence.h"

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace art {

  class FileCatalogMetadata {
  public:
    using collection_type = std::vector<std::pair<std::string, std::string>>;
    using value_type = collection_type::value_type;

    struct Config {
      fhicl::Atom<bool> checkSyntax{fhicl::Name("checkSyntax"), false};
      fhicl::OptionalAtom<std::string> applicationFamily{
        fhicl::Name("applicationFamily")};
      fhicl::OptionalAtom<std::string> applicationVersion{
        fhicl::Name("applicationVersion")};
      fhicl::OptionalAtom<std::string> group{fhicl::Name("group")};
      fhicl::OptionalAtom<std::string> processID{fhicl::Name("processID")};

      fhicl::Sequence<std::string> metadataFromInput{
        fhicl::Name("metadataFromInput"),
        fhicl::Comment(
          "This list specifies the metadata that is inherited\n"
          "from the input file.  Currently only the run type and\n"
          "file type metadata can be inherited.  The default list is empty."),
        std::vector<std::string>{}};

      bool
      inMetadataList(std::string const& name) const
      {
        return cet::search_all(metadataFromInput(), name);
      }

      fhicl::Atom<std::string> fileType{
        fhicl::Name("fileType"),
        fhicl::Comment("Can specify 'fileType' only if it is not specified\n"
                       "in the 'metadataFromInput' list."),
        [this] { return !inMetadataList("fileType"); },
        "unknown"};

      fhicl::OptionalAtom<std::string> runType{
        fhicl::Name("runType"),
        fhicl::Comment("Can specify 'runType' only if it is not specified\n"
                       "in the 'metadataFromInput' list."),
        [this] { return !inMetadataList("runType"); }};
    };

    using Parameters = ServiceTable<Config>;

    explicit FileCatalogMetadata(Parameters const& config);

    // Add a new value to the metadata store.
    void addMetadata(std::string const& key, std::string const& value);

    // Ensure the value is a canonical string representation.
    void addMetadataString(std::string const& key, std::string const& value);

    // Dump stored metadata into the provided container.
    void getMetadata(collection_type& coll) const;

    // RootInput can set the run-type and file-type parameters
    void setMetadataFromInput(collection_type const& coll);

    // Ascertain whether JSON syntax checking is desired.
    bool wantCheckSyntax() const noexcept;

    // Types
  private:
    class InheritedMetadata {
    public:
      InheritedMetadata(std::vector<std::string> const& sortedMdToInherit,
                        collection_type const& coll)
      {
        detail::OldToNew const translator;
        for (auto const& pr : coll) {
          if (cet::search_all(sortedMdToInherit, translator(pr.first))) {
            inputmd_.insert(pr);
            orderedmd_.emplace_back(pr);
          }
        }
      }

      auto const&
      entries() const
      {
        return orderedmd_;
      }

      void
      check_values(collection_type const& fromInput) const
      {
        for (auto const& [key, value] : fromInput) {
          auto it = inputmd_.find(key);
          if (it == cend(inputmd_)) {
            throw Exception(errors::LogicError)
              << "Metadata key " << key
              << " missing from list of metadata to inherit from input "
                 "files.\n";
          } else if (it->second != value) {
            throw Exception(errors::MismatchedInputFiles)
              << "The value for '" << key
              << "' for the current file is: " << value
              << ", which conflicts with the value from the first input file "
                 "(\""
              << it->second << "\").\n";
          }
        }
      }

    private:
      collection_type orderedmd_;
      std::unordered_map<std::string, std::string> inputmd_;
    };

  private:
    // Protects all data members.
    mutable std::recursive_mutex mutex_{};

    // Whether or not the user wishes metadata to be checked for syntax by
    // parsing with a JSON parser.
    bool const checkSyntax_;

    // Metadata the user wishes to inherit from the input file, const after the
    // ctor.
    std::vector<std::string> const mdToInherit_;

    // The collected metadata.
    collection_type md_{};

    // Metadata we have already inherited.
    std::unique_ptr<InheritedMetadata> imd_{};
  };

} // namespace art

DECLARE_ART_SERVICE(art::FileCatalogMetadata, SHARED)

#endif /* art_Framework_Services_System_FileCatalogMetadata_h */

// Local Variables:
// mode: c++
// End:
