#include "art/Framework/Art/detail/PrintPluginMetadata.h"
#include "art/Utilities/PluginSuffixes.h"

int main()
{
  for (auto const& pr : art::Suffixes::all())
    art::detail::print_available_plugins(pr.first, true, "art/.*");

  // Message facility is special
  art::detail::print_descriptions({"message"});
}
