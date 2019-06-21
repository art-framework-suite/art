#include "art/Framework/Core/ConsumesCollector.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchType.h"

using namespace std;

namespace art {

  array<vector<ProductInfo>, NumBranchTypes> const&
  ConsumesCollector::getConsumables() const
  {
    return consumables_;
  }

  void
  ConsumesCollector::sortConsumables(std::string const& current_process_name)
  {
    // Now that we know we have seen all the consumes declarations,
    // sort the results for fast lookup later.
    for (auto& perBTConsumables : consumables_) {
      for (auto& pi : perBTConsumables) {
        pi.process = ProcessTag{pi.process.name(), current_process_name};
      }
      sort(begin(perBTConsumables), end(perBTConsumables));
    }
  }

} // namespace art
