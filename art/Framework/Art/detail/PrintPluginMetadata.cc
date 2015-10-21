#include "art/Framework/Art/detail/PrintFormatting.h"
#include "art/Framework/Art/detail/PrintPluginMetadata.h"
#include "art/Framework/Art/detail/get_MetadataCollector.h"
#include "art/Framework/Art/detail/get_MetadataSummary.h"
#include "art/Utilities/PluginSuffixes.h"

#include <iostream>

using namespace art::detail;
using std::cout;

namespace {

  std::vector<art::detail::PluginMetadata>
  matchesBySpec(std::string const& spec)
  {
    std::vector<PluginMetadata> result;
    for ( auto const & pr : art::Suffixes::all() ) {
      auto mc = get_MetadataCollector(pr.first);

      for ( auto const& info : get_LibraryInfoCollection(pr.first, spec, indent__2()) )
        result.push_back( mc->collect(info) );
    }
    return result;
  }

}

void
art::detail::print_available_plugins(suffix_type st,
                                     std::string const& spec)
{
  auto coll = get_LibraryInfoCollection(st, spec);
  if ( coll.empty() ) return;

  auto ms = get_MetadataSummary(st, coll);

  cout << "\n" << thick_rule(ms->widths()) << "\n\n"
       << ms->header()
       << "\n" << thin_rule(ms->widths()) << "\n";
  for ( auto const & info : coll ) {
    cout << ms->summary(info);
  }
  cout << "\n" << thick_rule(ms->widths()) << "\n\n";
}

void
art::detail::print_description(std::vector<PluginMetadata> const& matches)
{
  for (auto const& m : matches) {
    cout << m.header()
         << m.details()
         << m.allowed_configuration();
    cout << fixed_rule();
  }
}

void
art::detail::print_descriptions(std::vector<std::string> const& plugins)
{
  cout << fixed_rule();
  for (auto const& plugin : plugins) {

    auto matches = matchesBySpec(plugin);
    if ( matches.empty() ) {
      cout << indent0() << font_bold(plugin) << " did not match any plugin.\n";
      cout << fixed_rule();
      continue;
    }

    print_description(matches);
  }
}
