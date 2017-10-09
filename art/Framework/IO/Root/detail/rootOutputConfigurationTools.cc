#include "art/Framework/IO/Root/detail/rootOutputConfigurationTools.h"
#include "art/Framework/Core/OutputFileGranularity.h"
#include "art/Framework/IO/Root/RootOutputClosingCriteria.h"
#include "canvas/Utilities/Exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

using namespace art;

namespace {
  bool
  maxCriterionSpecified(ClosingCriteria const& cc)
  {
    auto fp = std::mem_fn(&ClosingCriteria::fileProperties);
    return fp(cc).nEvents() != Defaults::unsigned_max() ||
           fp(cc).nSubRuns() != Defaults::unsigned_max() ||
           fp(cc).nRuns() != Defaults::unsigned_max() ||
           fp(cc).size() != Defaults::size_max() ||
           fp(cc).age().count() != Defaults::seconds_max();
  }
}

bool
art::detail::shouldFastClone(bool const fastCloningSet,
                             bool const fastCloning,
                             bool const wantAllEvents,
                             ClosingCriteria const& cc)
{
  bool result{fastCloning};
  mf::LogInfo("FastCloning")
    << "Initial fast cloning configuration "
    << (fastCloningSet ? "(user-set): " : "(from default): ") << std::boolalpha
    << fastCloning;

  if (fastCloning && !wantAllEvents) {
    result = false;
    mf::LogWarning("FastCloning")
      << "Fast cloning deactivated due to presence of\n"
      << "event selection configuration.";
  }
  if (fastCloning && maxCriterionSpecified(cc) &&
      cc.granularity() < Granularity::InputFile) {
    result = false;
    mf::LogWarning("FastCloning")
      << "Fast cloning deactivated due to request to allow\n"
      << "output file switching at an Event, SubRun, or Run boundary.";
  }
  return result;
}

bool
art::detail::shouldDropEvents(bool const dropAllEventsSet,
                              bool const dropAllEvents,
                              bool const dropAllSubRuns)
{
  if (!dropAllSubRuns)
    return dropAllEvents;

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
art::detail::validateFileNamePattern(bool const do_check,
                                     std::string const& pattern)
{
  if (!do_check)
    return;

  if (pattern.find("%#") == std::string::npos)
    throw Exception(errors::Configuration)
      << "If you have specified the 'fileProperties' table in a RootOutput "
         "module configuration,\n"
      << "then the file pattern '%#' MUST be present in the file name.  For "
         "example:\n"
      << "    " << pattern.substr(0, pattern.find(".root")) << "_%#.root\n"
      << "is a supported file name.  Please change your file name to include "
         "the '%#' pattern.";
}
