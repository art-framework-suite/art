#include "art/Framework/IO/Root/RootOutputClosingCriteria.h"
#include "art/Framework/IO/Root/detail/rootOutputConfigurationTools.h"
#include "canvas/Utilities/Exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

bool
art::detail::shouldFastClone(bool const fastCloningSet,
                             bool const fastCloning,
                             bool const wantAllEvents,
                             ClosingCriteria const& cc)
{
  bool result {fastCloning};
  mf::LogInfo("FastCloning") << "Initial fast cloning configuration "
                             << (fastCloningSet ? "(user-set): " : "(from default): ")
                             << std::boolalpha << fastCloning;

  if (fastCloning && !wantAllEvents) {
    result = false;
    mf::LogWarning("FastCloning") << "Fast cloning deactivated due to presence of\n"
                                  << "event selection configuration.";
  }
  if (fastCloning && cc.granularity() < Boundary::InputFile) {
    result = false;
    mf::LogWarning("FastCloning") << "Fast cloning deactivated due to request to allow\n"
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
