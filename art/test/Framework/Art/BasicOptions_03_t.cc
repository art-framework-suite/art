#include "art/Framework/Art/detail/AllowedConfiguration.h"
#include "art/Utilities/PluginSuffixes.h"

int
main() try {
  for (auto const& suffix : art::Suffixes::all()) {
    art::detail::print_available_plugins(
      suffix, "Dummy.*", true);
  }

  // Message facility is special
  art::detail::print_descriptions({"message"});
}
catch (...) {
  std::cout << " Exception thrown.";
  return 1;
}
