#include "art/Framework/Art/detail/PrintFormatting.h"
#include "art/Framework/Art/detail/AllowedConfiguration.h"
#include "art/Framework/Art/detail/get_MetadataCollector.h"
#include "art/Framework/Art/detail/get_MetadataSummary.h"
#include "cetlib/HorizontalRule.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Utilities/bold_fontify.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/detail/SearchAllowedConfiguration.h"

#include <iomanip>
#include <iostream>

using namespace art::detail;
using std::cout;

namespace {

  constexpr cet::HorizontalRule fixed_rule{100};

  std::vector<art::detail::PluginMetadata> matchesBySpec(std::string const& spec)
  {
    std::vector<PluginMetadata> result;
    for (auto const& pr : art::Suffixes::all()) {
      auto mc = get_MetadataCollector(pr.first);
      cet::transform_all(get_LibraryInfoCollection(pr.first, spec),
                         std::back_inserter(result),
                         [&mc](auto const& info){
                           return mc->collect(info, indent__2());
                         });
    }
    return result;
  }

  using Duplicates_t = std::map<std::string, std::vector<std::string>>;
  void duplicates_message(art::suffix_type const st, Duplicates_t const& duplicates)
  {
    using namespace art;
    std::string const type_spec = (st==suffix_type::plugin) ? "plugin_type" : "module_type";
    cout << indent0() << "The " << Suffixes::get(st) << "s marked '*' above are degenerate--i.e. specifying the short\n"
         << indent0() << type_spec << " value leads to an ambiguity.  In order to use a degenerate\n"
         << indent0() << Suffixes::get(st) << ", in your configuration file, give the long specification (as\n"
         << indent0() << "shown in the table below), surrounded by quotation (\") marks.\n\n";
    std::size_t const firstColW {columnWidth(duplicates, &Duplicates_t::value_type::first, "module_type")};
    cout << indent0()
         << std::setw(firstColW+4) << std::left << type_spec
         << std::left << "Long specification" << '\n';
    cout << indent0() << fixed_rule('-') << '\n';
    for (auto const& dup : duplicates) {
      auto const& long_specs = dup.second;
      cout << indent0()
           << std::setw(firstColW+4) << std::left << dup.first
           << std::left << long_specs[0] << '\n';
      for (auto it = long_specs.begin()+1, end = long_specs.end(); it != end; ++it)  {
        cout << indent0()
             << std::setw(firstColW+4) << "\"\""
             << std::left << *it << '\n';
      }
    }
  }

}

void
art::detail::print_available_plugins(suffix_type const st,
                                     bool const verbose,
                                     std::string const& spec)
{
  auto coll = get_LibraryInfoCollection(st, spec, verbose);
  if (coll.empty()) return;

  auto ms = get_MetadataSummary(st, coll);

  cet::HorizontalRule const rule{rule_size(ms->widths())};
  cout << "\n" << rule('=') << "\n\n"
       << ms->header()
       << "\n" << rule('-') << '\n';

  std::size_t i {};
  Duplicates_t duplicates;
  for (auto const& info : coll) {
    auto summary = ms->summary(info, ++i);
    cout << summary.message;
    if (summary.is_duplicate)
      duplicates[info.short_spec()].push_back(info.long_spec());
  }
  cout << "\n" << rule('=') << "\n\n";

  if (duplicates.empty()) return;

  duplicates_message(st, duplicates);
  cout << "\n\n";
}

bool
art::detail::supports_key(suffix_type const st, std::string const& spec, std::string const& key [[gnu::unused]])
{
  art::Exception e {art::errors::LogicError, "art::detail::has_key"};
  auto coll = get_LibraryInfoCollection(st, spec);
  if (coll.empty()) {
    throw e << bold_fontify(spec) << " did not match any plugin.\n";
  }
  else if (coll.size() > 1ull) {
    throw e << bold_fontify(spec) << " matched more than one plugin.\n"
            << "When querying plugin configurations, the plugin specification\n"
            << "must resolve to a unique library.\n";
  }
  if (auto config = coll.begin()->allowed_config()) {
    return fhicl::detail::SearchAllowedConfiguration::supports_key(*config->parameter_base(), key);
  }
  return false;
}

void
art::detail::print_description(std::vector<PluginMetadata> const& matches)
{
  for (auto const& m : matches) {
    cout << m.header()
         << m.details()
         << m.allowed_configuration();
    cout << '\n' << fixed_rule('=') << "\n\n";
  }
}

void
art::detail::print_descriptions(std::vector<std::string> const& plugins)
{
  cout << '\n' << fixed_rule('=') << "\n\n";
  for (auto const& plugin : plugins) {
    auto matches = matchesBySpec(plugin);
    if (matches.empty()) {
      cout << indent0() << bold_fontify(plugin) << " did not match any plugin.\n";
      cout << '\n' << fixed_rule('=') << "\n\n";
      continue;
    }
    print_description(matches);
  }
}
