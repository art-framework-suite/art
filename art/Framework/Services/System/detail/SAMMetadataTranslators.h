#ifndef art_Framework_Services_System_detail_SAMMetadataTranslators_h
#define art_Framework_Services_System_detail_SAMMetadataTranslators_h

#include <map>
#include <string>

namespace art {
  namespace detail {

    using new_t = std::string;
    using old_t = std::string;

    inline std::map<old_t, new_t>
    oldToNewName()
    {
      return {{"fileType", "file_type"},
              {"dataTier", "data_tier"},
              {"streamName", "data_stream"},
              {"runType", "run_type"}};
    }

    //==============================================================================

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

  } // namespace detail
} // namespace art

#endif /* art_Framework_Services_System_detail_SAMMetadataTranslators_h */

// Local variables:
// mode: c++
// End:
