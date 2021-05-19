#include "art/Framework/Art/detail/AllowedConfiguration.h"
#include "art/Framework/Art/detail/PrintFormatting.h"
#include "art/Framework/Art/detail/get_MetadataCollector.h"
#include "art/Framework/Art/detail/get_MetadataSummary.h"
#include "art/Utilities/PluginSuffixes.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/bold_fontify.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/detail/SearchAllowedConfiguration.h"

#include <iomanip>
#include <iostream>
#include <tuple>

using namespace art::detail;
using std::cout;

namespace {

  std::map<std::string,
           std::pair<std::string, std::string>> const block_parameters = {
    {art::Suffixes::module(), {"module_type", "module_type"}},
    {art::Suffixes::service(), {"", ""}},
    {art::Suffixes::source(), {"module_type", "module_type"}},
    {art::Suffixes::mfPlugin(), {"Destination type ", "type"}},
    {art::Suffixes::mfStatsPlugin(), {"Statistics destination type ", "type"}},
    {art::Suffixes::plugin(), {"plugin_type", "plugin_type"}},
    {art::Suffixes::tool(), {"tool_type", "tool_type"}}};

  constexpr cet::HorizontalRule fixed_rule{100};

  std::vector<art::detail::PluginMetadata>
  matchesBySpec(std::string const& specified_plugin_type,
                std::string const& instance_pattern)
  {
    std::vector<PluginMetadata> result;
    auto collect_metadata = [&result,
                             &instance_pattern](std::string const& suffix) {
      auto mc = get_MetadataCollector(suffix);
      // FIXME: Should the header label and and param_to_replace have
      // defaults?
      std::string header_label{"plugin_type"};
      std::string param_to_replace{"plugin_type"};
      auto params = block_parameters.find(suffix);
      if (params != cend(block_parameters)) {
        header_label = params->second.first;
        param_to_replace = params->second.second;
      }
      cet::transform_all(
        get_LibraryInfoCollection(suffix, instance_pattern),
        back_inserter(result),
        [&mc, &header_label, &param_to_replace](auto const& info) {
          return mc->collect(info, indent__2(), header_label, param_to_replace);
        });
    };

    if (specified_plugin_type.empty()) {
      // Search through all plugin types known to art if the user has
      // not specified one.
      for (auto const& suffix : art::Suffixes::all()) {
        collect_metadata(suffix);
      }
    } else {
      collect_metadata(specified_plugin_type);
    }
    return result;
  }

  using Duplicates_t = std::map<std::string, std::vector<std::string>>;
  void
  duplicates_message(std::string const& suffix, Duplicates_t const& duplicates)
  {
    using namespace art;
    std::string const type_spec =
      (suffix == Suffixes::plugin()) ? "plugin_type" : "module_type";
    cout
      << indent0() << "The " << suffix
      << "s marked '*' above are degenerate--i.e. specifying the short\n"
      << indent0() << type_spec
      << " value leads to an ambiguity.  In order to use a degenerate\n"
      << indent0() << suffix
      << ", in your configuration file, give the long specification (as\n"
      << indent0()
      << "shown in the table below), surrounded by quotation (\") marks.\n\n";
    std::size_t const firstColW{
      columnWidth(duplicates, &Duplicates_t::value_type::first, "module_type")};
    cout << indent0() << std::setw(firstColW + 4) << std::left << type_spec
         << std::left << "Long specification" << '\n';
    cout << indent0() << fixed_rule('-') << '\n';
    for (auto const& dup : duplicates) {
      auto const& long_specs = dup.second;
      cout << indent0() << std::setw(firstColW + 4) << std::left << dup.first
           << std::left << long_specs[0] << '\n';
      for (auto it = long_specs.begin() + 1, end = long_specs.end(); it != end;
           ++it) {
        cout << indent0() << std::setw(firstColW + 4) << "\"\"" << std::left
             << *it << '\n';
      }
    }
  }

} // namespace

void
art::detail::print_available_plugins(std::string const& suffix,
                                     std::string const& spec,
                                     bool const verbose)
{
  auto coll = get_LibraryInfoCollection(suffix, spec, verbose);
  if (coll.empty()) {
    cout << "Unable to find any plugins with suffix '" << suffix << "'.\n";
    return;
  }

  auto ms = get_MetadataSummary(suffix, coll);

  cet::HorizontalRule const rule{rule_size(ms->widths())};
  cout << '\n'
       << rule('=') << '\n'
       << ms->header() << '\n'
       << rule('-') << '\n';

  std::size_t i{};
  Duplicates_t duplicates;
  for (auto const& info : coll) {
    auto summary = ms->summary(info, ++i);
    cout << summary.message;
    if (summary.is_duplicate)
      duplicates[info.short_spec()].push_back(info.long_spec());
  }
  cout << rule('=') << "\n\n";

  if (duplicates.empty())
    return;

  duplicates_message(suffix, duplicates);
  cout << '\n';
}

bool
art::detail::supports_key(std::string const& suffix,
                          std::string const& spec,
                          std::string const& key)
{
  art::Exception e{art::errors::LogicError, "art::detail::supports_key"};
  auto coll = get_LibraryInfoCollection(suffix, spec);
  if (coll.empty()) {
    throw e << (spec.empty() ? "[Missing specification]" :
                               cet::bold_fontify(spec))
            << " did not match any plugin.\n";
  } else if (coll.size() > 1ull) {
    throw e << cet::bold_fontify(spec) << " matched more than one plugin.\n"
            << "When querying plugin configurations, the plugin specification\n"
            << "must resolve to a unique library.\n";
  }
  if (auto config = coll.begin()->allowed_config()) {
    return fhicl::detail::SearchAllowedConfiguration::supports_key(
      *config->parameter_base(), key);
  }
  return false;
}

void
art::detail::print_description(std::vector<PluginMetadata> const& matches)
{
  for (auto const& m : matches) {
    cout << m.header() << m.details() << m.allowed_configuration();
    cout << '\n' << fixed_rule('=') << "\n\n";
  }
}

namespace {
  std::pair<std::string, std::string>
  parse_specified_plugin(std::string const& spec)
  {
    // The specified plugin can be of the pattern:
    //
    //   [<plugin_type>:]<regex corresponding to plugin instance>
    //
    // If '<plugin_type>:' is omitted, then all plugin types are searched.
    std::string specified_plugin_type{};
    std::string instance_pattern{spec};
    auto const pos = spec.find(":");
    if (pos != std::string::npos) {
      specified_plugin_type = spec.substr(0, pos);
      if (specified_plugin_type.empty()) {
        throw art::Exception{art::errors::Configuration,
                             "Error while parsing specified plugins:\n"}
          << "The specification '" << spec
          << "' is missing a module type before the colon (':').\n"
             "If you intend to search through all plugin types, remove the "
             "colon; otherwise specify\n"
             "one of the following plugin types:"
          << art::Suffixes::print() << '\n';
      }
      instance_pattern = spec.substr(pos + 1);
    }
    return std::make_pair(move(specified_plugin_type), move(instance_pattern));
  }
} // namespace

void
art::detail::print_descriptions(std::vector<std::string> const& specs)
{
  cout << '\n' << fixed_rule('=') << "\n\n";
  for (auto const& spec : specs) {
    auto const [plugin_type, instance_pattern] = parse_specified_plugin(spec);

    auto matches = matchesBySpec(plugin_type, instance_pattern);
    if (matches.empty()) {
      cout << indent0()
           << (instance_pattern.empty() ? "[Missing specification]" :
                                          cet::bold_fontify(instance_pattern))
           << " did not match any plugin";
      cout << (plugin_type.empty() ? "" : " of type '" + plugin_type + "'");
      cout << ".\n";
      cout << '\n' << fixed_rule('=') << "\n\n";
      continue;
    }
    print_description(matches);
  }
}
