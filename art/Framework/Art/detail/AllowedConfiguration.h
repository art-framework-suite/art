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
#include "art/Utilities/PluginSuffixes.h"

#include <string>
#include <vector>

namespace art {
  namespace detail {

    void print_available_plugins(suffix_type st,
                                 bool const verbose,
                                 std::string const& spec);

    inline void
    print_available_plugins(suffix_type st, bool const verbose)
    {
      print_available_plugins(st, verbose, dflt_spec_pattern());
    }

    bool supports_key(suffix_type st,
                      std::string const& spec,
                      std::string const& key);
    void print_description(std::vector<PluginMetadata> const& matches);
    void print_descriptions(std::vector<std::string> const& plugins);

  } // namespace detail
} // namespace art

#endif /* art_Framework_Art_detail_AllowedConfiguration_h */

// Local variables:
// mode: c++
// End:
