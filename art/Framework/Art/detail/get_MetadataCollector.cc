#include "art/Framework/Art/detail/get_MetadataCollector.h"
#include "art/Framework/Art/detail/md-collector/MetadataCollectorForModule.h"
#include "art/Framework/Art/detail/md-collector/MetadataCollectorForPlugin.h"
#include "art/Framework/Art/detail/md-collector/MetadataCollectorForService.h"
#include "art/Framework/Art/detail/md-collector/MetadataCollectorForSource.h"
#include "art/Framework/Art/detail/md-collector/MetadataCollectorForTool.h"
#include "canvas/Utilities/Exception.h"

using namespace art;
using namespace art::detail;

std::unique_ptr<MetadataCollector>
detail::get_MetadataCollector(suffix_type const st)
{
  switch(st) {
  case suffix_type::module : return std::make_unique<MetadataCollectorFor<suffix_type::module> >();
  case suffix_type::plugin : return std::make_unique<MetadataCollectorFor<suffix_type::plugin> >();
  case suffix_type::service: return std::make_unique<MetadataCollectorFor<suffix_type::service>>();
  case suffix_type::source : return std::make_unique<MetadataCollectorFor<suffix_type::source> >();
  case suffix_type::tool   : return std::make_unique<MetadataCollectorFor<suffix_type::tool>   >();
    // No default -- allow compiler to warn if missing suffix_type.
  }
  return std::unique_ptr<MetadataCollector>{nullptr};
}
