#include "art/Utilities/PluginSuffixes.h"

namespace {
  auto
  reverseMap(std::map<art::suffix_type, std::string> const& map)
  {
    std::map<std::string, art::suffix_type> result;
    for (auto const& pr : map)
      result.emplace(pr.second, pr.first);
    return result;
  }
} // namespace

std::map<art::suffix_type, std::string> art::Suffixes::suffixes_ = {
  {suffix_type::module, "module"},
  {suffix_type::plugin, "plugin"},
  {suffix_type::service, "service"},
  {suffix_type::source, "source"},
  {suffix_type::tool, "tool"},
  {suffix_type::mfPlugin, "mfPlugin"},
  {suffix_type::mfStatsPlugin, "mfStatsPlugin"}};

std::map<std::string, art::suffix_type> art::Suffixes::rSuffixes_ =
  reverseMap(suffixes_);
