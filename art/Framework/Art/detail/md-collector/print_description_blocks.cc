#include "art/Framework/Art/detail/md-collector/print_description_blocks.h"
#include "art/Framework/Art/detail/MetadataRegexHelpers.h"
#include "art/Framework/Art/detail/PrintFormatting.h"
#include "art/Framework/Art/detail/md-collector/describe.h"
#include "art/Utilities/bold_fontify.h"
#include "cetlib/LibraryManager.h"

#include <sstream>

namespace art {
  namespace detail {
    std::string
    print_header(LibraryInfo const& li, std::string const& type_spec)
    {
      std::ostringstream result;
      std::string const long_spec =
        li.long_spec().empty() ? " [ No alternate specification available ] " :
                                 li.long_spec();
      result << indent_1() << type_spec << ": " << bold_fontify(li.short_spec())
             << " (or \"" << long_spec << "\")"
             << "\n\n";
      return result.str();
    }

    std::string
    print_allowed_configuration(LibraryInfo const& li,
                                std::string const& prefix,
                                std::string const& type_spec)
    {
      std::ostringstream result;
      result << indent_1() << "Allowed configuration\n"
             << indent_1() << "---------------------\n";

      std::string printedConfig{describe(li.allowed_config(), prefix)};
      if (!type_spec.empty()) {
        replace_type(printedConfig, li.short_spec(), regex_for_spec(type_spec));
      }
      result << printedConfig;
      return result.str();
    }
  }
}
