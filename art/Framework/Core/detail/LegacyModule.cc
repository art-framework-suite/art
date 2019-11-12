#include "art/Framework/Core/detail/LegacyModule.h"
#include "art/Utilities/SharedResourcesRegistry.h"

namespace art::detail {
  LegacyModule::LegacyModule(std::string const& module_label)
    : EngineCreator{module_label, ScheduleID::first()}
  {
    serialize(SharedResourcesRegistry::Legacy);
  }

  ScheduleID
  LegacyModule::scheduleID() const noexcept
  {
    return scheduleID_.load();
  }

  void
  LegacyModule::setScheduleID(ScheduleID const sid) noexcept
  {
    scheduleID_ = sid;
  }

  LegacyModule::ScheduleIDSentry::ScheduleIDSentry(
    LegacyModule& mod,
    ScheduleID const sid) noexcept
    : mod_{mod}
  {
    mod_.setScheduleID(sid);
  }

  LegacyModule::ScheduleIDSentry::~ScheduleIDSentry() noexcept
  {
    mod_.setScheduleID(ScheduleID{});
  }
}
