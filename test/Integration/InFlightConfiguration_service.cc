#include "boost/test/included/unit_test.hpp"

#include "test/Integration/InFlightConfiguration.h"

#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/PathSelection.h"

#include <iomanip>
#include <iostream>

arttest::InFlightConfiguration::
InFlightConfiguration(fhicl::ParameterSet const & ps,
                      art::ActivityRegistry & areg)
:
  UserInteraction(areg),
  moduleInfos_(),
  pathSelectionService_(),
  callCount_(0)
{
  if (ps.get("test_missing_pathselection", false)) {
    BOOST_REQUIRE(!art::ServiceRegistry::instance().isAvailable<art::PathSelection>());
  } else {
    pathSelectionService_.reset(new art::ServiceHandle<art::PathSelection>);
  }
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
  if (!pathSelectionService_) {
    return;
  }
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
  //   (*pathSelectionService_)->setEndPathModuleEnabled(label, bool);
  //
  // Similarly, disablement of trigger paths may be done with:
  //
  //   (*pathSelectionService_)->setTriggerPathEnabled(name, bool);
  //
  // Analogous with the previous function, an attempt to disable the end
  // path (or a nonexistent path) will result in an exception.
  //
  //  In both cases, the return value from the function is the
  // *previous* enablement state of the specified module.
  ++callCount_;
  if (callCount_ == 1) {
    // Deactivate path.
    (*pathSelectionService_)->setTriggerPathEnabled("p2", false);
    // Deactivate module.
    (*pathSelectionService_)->setEndPathModuleEnabled("ia2", false);
  } else {
    // Reconfigure analyzer.
    auto it = std::find_if(moduleInfos_.cbegin(),
                           moduleInfos_.cend(),
                           [](ModuleInfo const & mi)
                           {
                             return mi.label == "ia1";
                           });
    BOOST_REQUIRE(it != moduleInfos_.cend());
    BOOST_REQUIRE(it->endPathModule);
    auto ps = it->pset;
    ps.erase("require_presence"); // See issue #4452.
    ps.put("require_presence", false);
    std::cerr << "require_presence: "
              << std::boolalpha
              << ps.get<bool>("require_presence")
              << ".\n";
    callReconfigure(std::distance(moduleInfos_.cbegin(), it), ps);
    // Deactivate path.
    (*pathSelectionService_)->setTriggerPathEnabled("p1", false);
    // Reactivate other path.
    (*pathSelectionService_)->setTriggerPathEnabled("p2", true);
    // Reactivate end path module.
    (*pathSelectionService_)->setEndPathModuleEnabled("ia2", true);
  }
}

auto
arttest::InFlightConfiguration::
nextAction()
-> NextStep
{
  return NextEvent;
}

DEFINE_ART_SERVICE(arttest::InFlightConfiguration)
