#include "art/Framework/Core/detail/LegacyModule.h"

namespace art {
  namespace detail {

    LegacyModule::LegacyModule() = default;
    LegacyModule::LegacyModule(std::string const& module_label)
      : EngineCreator{module_label, ScheduleID::first()}
    {}

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
}
