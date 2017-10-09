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
detail::get_MetadataSummary(suffix_type const st,
                            LibraryInfoCollection const& coll)
{
  switch (st) {
    case suffix_type::module:
      return std::make_unique<MetadataSummaryFor<suffix_type::module>>(coll);
    case suffix_type::service:
      return std::make_unique<MetadataSummaryFor<suffix_type::service>>(coll);
    case suffix_type::source:
      return std::make_unique<MetadataSummaryFor<suffix_type::source>>(coll);
    case suffix_type::plugin:
      return std::make_unique<MetadataSummaryFor<suffix_type::plugin>>(coll);
    case suffix_type::tool:
      return std::make_unique<MetadataSummaryFor<suffix_type::tool>>(coll);
    case suffix_type::mfPlugin:
      return std::make_unique<MetadataSummaryFor<suffix_type::mfPlugin>>(coll);
    case suffix_type::mfStatsPlugin:
      return std::make_unique<MetadataSummaryFor<suffix_type::mfStatsPlugin>>(
        coll);
      // No default -- let compiler warn if missing type.
  }
  return std::unique_ptr<MetadataSummary>{nullptr};
}
