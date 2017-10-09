#include "art/Framework/IO/Root/detail/rootOutputConfigurationTools.h"
// vim: set sw=2 expandtab :

#include "canvas/Utilities/Exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

using namespace art;

namespace art {
  namespace detail {

    bool
    shouldDropEvents(bool const dropAllEventsSet,
                     bool const dropAllEvents,
                     bool const dropAllSubRuns)
    {
      if (!dropAllSubRuns) {
        return dropAllEvents;
      }
      if (dropAllEventsSet && !dropAllEvents) {
        throw art::Exception(errors::Configuration)
          << "\nThe following FHiCL specification is illegal\n\n"
          << "   dropAllEvents  : false \n"
          << "   dropAllSubRuns : true  \n\n"
          << "[1] Both can be 'true', "
          << "[2] both can be 'false', or "
          << "[3] 'dropAllEvents : true' and 'dropAllSubRuns : false' "
          << "is allowed.\n\n";
      }
      return true;
    }

    void
    validateFileNamePattern(bool const do_check, std::string const& pattern)
    {
      if (!do_check) {
        return;
      }
      if (pattern.find("%#") == std::string::npos) {
        throw Exception(errors::Configuration)
          << "If you have specified the 'fileProperties' table in a RootOutput "
             "module configuration,\n"
          << "then the file pattern '%#' MUST be present in the file name.  "
             "For "
             "example:\n"
          << "    " << pattern.substr(0, pattern.find(".root")) << "_%#.root\n"
          << "is a supported file name.  Please change your file name to "
             "include "
             "the '%#' pattern.";
      }
    }

  } // namespace detail
} // namespace art
