#include "test/Integration/InFlightConfiguration.h"

#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/PathSelection.h"

arttest::InFlightConfiguration::
InFlightConfiguration(fhicl::ParameterSet const &,
                      art::ActivityRegistry & areg)
:
  UserInteraction(areg),
  moduleInfos_(),
  pathSelectionService_()
{
}

void
arttest::InFlightConfiguration::
moduleList(std::vector<ModuleInfo> const & infos)
{
  moduleInfos_ = infos;
}

void
arttest::InFlightConfiguration::
pickModule()
{
  // Receive messages here and decide what to do.
  //
  // The moduleInfos_ container will list all configured modules. In
  // order to reconfigure a module, you will need to identify the
  // correct module, come up with a new parameterSet (the
  // originally-configured one is in the infos you were give) and
  // trigger the reconfiguration with:
  //
  //   callReconfigure(index, ps).
  //
  // Note that the module so identified *must* have implemented the
  // reconfigure(fhicl::ParameterSet const &) call or an exception will
  // be thrown that should be caught and dealt with here.
  //
  // As far as actually disabling end path modules, you will need to
  // decide which one(s) may be disabled. A check against
  // ModuleInfo::endPathModule would also be appropriate. An attempt to
  // disable a non-endPath (or nonexistent) module will result in an
  // exception.
  //
  //   pathSelectionService_->setEndPathModuleEnabled(label, bool);
  //
  // Similarly, disablement of trigger paths may be done with:
  //
  //   pathSelectionService_->setTriggerPathEnabled(name, bool);
  //
  // Analogous with the previous function, an attempt to disable the end
  // path (or a nonexistent path) will result in an exception.
  //
  //  In both cases, the return value from the function is the
  // *previous* enablement state of the specified module.
}

auto
arttest::InFlightConfiguration::
nextAction()
-> NextStep
{
  return NextEvent;
}

DEFINE_ART_SERVICE(arttest::InFlightConfiguration)
