#ifndef art_Framework_Art_detail_prune_configuration_h
#define art_Framework_Art_detail_prune_configuration_h

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace fhicl {
  class intermediate_table;
}

namespace art::detail {
  using modules_per_path_t = std::map<std::string, std::vector<std::string>>;
  using modules_t = std::map<std::string, std::string>;

  std::pair<modules_per_path_t, modules_t> detect_unused_configuration(
    fhicl::intermediate_table& config);

  void prune_configuration(modules_per_path_t const& unused_paths,
                           modules_t const& unused_modules,
                           fhicl::intermediate_table& config);
}

#endif /* art_Framework_Art_detail_prune_configuration_h */

// Local Variables:
// mode: c++
// End:
