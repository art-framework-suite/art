#ifndef art_Utilities_SAMMetadataTranslators_h
#define art_Utilities_SAMMetadataTranslators_h

#include "cetlib/container_algorithms.h"

#include <map>
#include <string>

namespace art {

  using new_t = std::string;
  using old_t = std::string;

  inline std::map<new_t, old_t>
  newToOldName()
  {
    return {{"file_type", "fileType"},
            {"data_tier", "dataTier"},
            {"data_stream", "streamName"},
            {"run_type", "runType"}};
  }

  // Translator
  struct NewToOld {
    std::string
    operator()(std::string const& name) const
    {
      auto const& transMap = newToOldName();
      auto it = transMap.find(name);
      return it != transMap.cend() ? it->second : name;
    }
  };

  //==============================================================================

  inline std::map<new_t, old_t>
  oldToNewName()
  {
    auto const newToOld = newToOldName();
    std::map<old_t, new_t> oldToNew;
    cet::transform_all(
      newToOld, std::inserter(oldToNew, oldToNew.begin()), [](auto const& pr) {
        return std::make_pair(pr.second, pr.first);
      });
    return oldToNew;
  }

  // Translator
  struct OldToNew {
    std::string
    operator()(std::string const& name) const
    {
      auto const& transMap = oldToNewName();
      auto it = transMap.find(name);
      return it != transMap.cend() ? it->second : name;
    }
  };
}

#endif /* art_Utilities_SAMMetadataTranslators_h */

// Local variables:
// mode: c++
// End:
