#include "art/Framework/Art/detail/get_MetadataCollector.h"
#include "art/Framework/Art/detail/md-collector/MetadataCollectorForMFPlugin.h"
#include "art/Framework/Art/detail/md-collector/MetadataCollectorForMFStatsPlugin.h"
#include "art/Framework/Art/detail/md-collector/MetadataCollectorForModule.h"
#include "art/Framework/Art/detail/md-collector/MetadataCollectorForPlugin.h"
#include "art/Framework/Art/detail/md-collector/MetadataCollectorForService.h"
#include "art/Framework/Art/detail/md-collector/MetadataCollectorForSource.h"
#include "art/Framework/Art/detail/md-collector/MetadataCollectorForTool.h"
#include "art/Utilities/PluginSuffixes.h"

using namespace art;
using namespace art::detail;

std::unique_ptr<MetadataCollector>
detail::get_MetadataCollector(std::string const& suffix)
{
  if (suffix == art::Suffixes::module()) {
    return std::make_unique<MetadataCollectorFor<suffix_type::module>>();
  }
  if (suffix == art::Suffixes::plugin()) {
    return std::make_unique<MetadataCollectorFor<suffix_type::plugin>>();
  }
  if (suffix == art::Suffixes::service()) {
    return std::make_unique<MetadataCollectorFor<suffix_type::service>>();
  }
  if (suffix == art::Suffixes::source()) {
    return std::make_unique<MetadataCollectorFor<suffix_type::source>>();
  }
  if (suffix == art::Suffixes::tool()) {
    return std::make_unique<MetadataCollectorFor<suffix_type::tool>>();
  }
  if (suffix == art::Suffixes::mfPlugin()) {
    return std::make_unique<MetadataCollectorFor<suffix_type::mfPlugin>>();
  }
  if (suffix == art::Suffixes::mfStatsPlugin()) {
    return std::make_unique<MetadataCollectorFor<suffix_type::mfStatsPlugin>>();
  }
  return std::unique_ptr<MetadataCollector>{nullptr};
}
