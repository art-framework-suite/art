#include "art/Framework/Art/detail/get_MetadataSummary.h"
#include "art/Framework/Art/detail/md-summary/MetadataSummaryForMFPlugin.h"
#include "art/Framework/Art/detail/md-summary/MetadataSummaryForMFStatsPlugin.h"
#include "art/Framework/Art/detail/md-summary/MetadataSummaryForModule.h"
#include "art/Framework/Art/detail/md-summary/MetadataSummaryForPlugin.h"
#include "art/Framework/Art/detail/md-summary/MetadataSummaryForService.h"
#include "art/Framework/Art/detail/md-summary/MetadataSummaryForSource.h"
#include "art/Framework/Art/detail/md-summary/MetadataSummaryForTool.h"
#include "canvas/Utilities/Exception.h"

using namespace art;
using namespace art::detail;

std::unique_ptr<MetadataSummary>
detail::get_MetadataSummary(std::string const& suffix,
                            LibraryInfoCollection const& coll)
{
  if (suffix == art::Suffixes::module()) {
    return std::make_unique<MetadataSummaryFor<suffix_type::module>>(coll);
  }
  if (suffix == art::Suffixes::plugin()) {
    return std::make_unique<MetadataSummaryFor<suffix_type::plugin>>(coll);
  }
  if (suffix == art::Suffixes::service()) {
    return std::make_unique<MetadataSummaryFor<suffix_type::service>>(coll);
  }
  if (suffix == art::Suffixes::source()) {
    return std::make_unique<MetadataSummaryFor<suffix_type::source>>(coll);
  }
  if (suffix == art::Suffixes::tool()) {
    return std::make_unique<MetadataSummaryFor<suffix_type::tool>>(coll);
  }
  if (suffix == art::Suffixes::mfPlugin()) {
    return std::make_unique<MetadataSummaryFor<suffix_type::mfPlugin>>(coll);
  }
  if (suffix == art::Suffixes::mfStatsPlugin()) {
    return std::make_unique<MetadataSummaryFor<suffix_type::mfStatsPlugin>>(
      coll);
  }
  return std::unique_ptr<MetadataSummary>{nullptr};
}
