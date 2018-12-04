#ifndef art_Framework_Modules_SampledInfo_h
#define art_Framework_Modules_SampledInfo_h

#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Utilities/Level.h"

#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace art {
  template <typename T>
  struct SampledInfo {
    static_assert(std::is_same<T, RunID>::value ||
                    std::is_same<T, SubRunID>::value,
                  "The template argument must be either RunID or SubRunID.");
    double weight;
    double probability;
    std::vector<T> ids;

    // MUST UPDATE WHEN CLASS IS CHANGED!
    static short
    Class_Version()
    {
      return 10;
    }
  };

  struct SampledEventInfo {
    EventID id;
    std::string dataset;
    double weight;
    double probability;
  };

  using SampledRunInfo = std::map<std::string, SampledInfo<RunID>>;
  using SampledSubRunInfo = std::map<std::string, SampledInfo<SubRunID>>;

  inline std::ostream&
  operator<<(std::ostream& os, SampledEventInfo const& eventInfo)
  {
    os << "Sampled EventID: '" << eventInfo.id << "'  Dataset: '"
       << eventInfo.dataset << "'  Weight: " << eventInfo.weight
       << "  Probability: " << eventInfo.probability;
    return os;
  }
}

#endif /* art_Framework_Modules_SampledInfo_h */

// Local Variables:
// mode: c++
// End:
