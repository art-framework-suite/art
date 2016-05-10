#include "art/Framework/IO/Root/detail/rootOutputConfigurationTools.h"
#include "canvas/Utilities/Exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

void
art::detail::checkFileSwitchConfig(Boundary const fileSwitchBoundary,
                                   bool const forceSwitch)
{
  if (fileSwitchBoundary == Boundary::Unset && forceSwitch) {
    throw art::Exception(art::errors::Configuration)
      << "The following configuration is invalid\n"
      << "   fileSwitch: {\n"
      << "     boundary: \"Unset\"\n"
      << "     force: true\n"
      << "   }\n"
      << "For output-file switching to be forced on a boundary, the boundary must be set.";
  }
}

art::Boundary
art::detail::checkMaxSizeConfig(bool const switchBoundarySet,
                                Boundary const switchBoundary,
                                bool const forceSwitch)
{
  if (forceSwitch) {
    throw art::Exception(art::errors::Configuration)
      << "The 'fileSwitch.force' parameter cannot be 'true'\n"
      << "whenever the 'maxSize' parameter is specified.";
  }
  return switchBoundarySet ? switchBoundary : Boundary::Event;
}

art::Boundary
art::detail::checkMaxAgeConfig(bool const switchBoundarySet,
                               Boundary const switchBoundary,
                               bool const forceSwitch)
{
  if (forceSwitch) {
    throw art::Exception(art::errors::Configuration)
      << "The 'fileSwitch.force' parameter cannot be 'true'\n"
      << "whenever the 'maxEventsPerFile' parameter is specified.";
  }
  return switchBoundarySet ? switchBoundary : Boundary::Event;
}

art::Boundary
art::detail::checkMaxEventsPerFileConfig(bool const switchBoundarySet,
                                         Boundary const switchBoundary,
                                         bool const forceSwitch)
{
  if (forceSwitch) {
    throw art::Exception(art::errors::Configuration)
      << "The 'fileSwitch.force' parameter cannot be 'true'\n"
      << "whenever the 'maxAge' parameter is specified.";
  }
  return switchBoundarySet ? switchBoundary : Boundary::Event;
}

bool
art::detail::shouldFastClone(bool const fastCloningSet,
                             bool const fastCloning,
                             bool const wantAllEvents,
                             Boundary const fileSwitchBoundary)
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
  if (fastCloning && fileSwitchBoundary < Boundary::InputFile) {
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
