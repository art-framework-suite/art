#ifndef art_Framework_Art_detail_AllowedConfiguration_h
#define art_Framework_Art_detail_AllowedConfiguration_h

/*
  The functions below:

      void print_available_plugins
      void print_description
      void print_descriptions
      bool supports_key

  are the driving functions for the art program options:

      art --print-available=(module|plugin|service|source)
      art --print-available-modules
      art --print-available-services
      art --print-description <specs>...

  ===============
  Developer notes
  ===============

  A primary design consideration for this system is that
  'PluginSuffixes' provides an enumerator for each supported plugin
  suffix.  This allows template programming to be used.  However, the
  necessity of looping over entries implies needing to use inheritance
  as well.

  For each plugin, the following information is stored as in a
  'LibraryInfo' object:

    - .so name
    - specs (long and short)
    - source file path
    - allowed configuration -- pointer to allowed configuration
    - provider ("art" or "user")
    - plugin type

  Some of the data members may be empty if the associated info is not
  relevant (e.g. plugin type for services).

  There are 3 get_* functions that are plugin-specific:

     1. get_LibraryInfoCollection(suffix_type)
     2. get_MetadataSummary  (suffix_type, LibraryInfoCollection const&)
     3. get_MetadataCollector(suffix_type)

  Function 1 is used for returning the LibraryInfo objects associated
  with a given plugin suffix.

  Function 2 returns the 'MetadataSummary', which is used for
  presenting one-line information for the --print-available* program
  options.

  Function 3 return the 'MetadataCollector', which is used for
  presenting more detailed information for '--print-description'.

  Both 'MetadataSummary' and 'MetadataCollector' are abstract base
  classes, whose derived classes are template instantiations of
  'MetadataSummaryFor<suffix_type>' or
  'MetadataCollectorFor<suffix_type>', respectively.

  For the '--print-description' option, the 'PluginMetadata' class is
  used to encapsulate the "header", "details", and "allowed
  configuration" of each plugin.

  KK, October 2015
*/

#include "art/Framework/Art/detail/PluginMetadata.h"
#include "art/Framework/Art/detail/get_LibraryInfoCollection.h"

#include <string>
#include <vector>

namespace art::detail {

  void print_available_plugins(std::string const& suffix,
                               std::string const& spec,
                               bool verbose);

  inline void
  print_available_plugins(std::string const& suffix, bool const verbose)
  {
    print_available_plugins(suffix, dflt_spec_pattern(), verbose);
  }

  bool supports_key(std::string const& suffix,
                    std::string const& spec,
                    std::string const& key);
  void print_description(std::vector<PluginMetadata> const& matches);
  void print_descriptions(std::vector<std::string> const& plugins);
}

#endif /* art_Framework_Art_detail_AllowedConfiguration_h */

// Local variables:
// mode: c++
// End:
