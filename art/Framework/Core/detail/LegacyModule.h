#ifndef art_Framework_Core_detail_LegacyModule_h
#define art_Framework_Core_detail_LegacyModule_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/detail/EngineCreator.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Utilities/ScheduleID.h"

#include <atomic>

namespace art::detail {

  class LegacyModule : public SharedModule, private EngineCreator {
  public:
    explicit LegacyModule();
    explicit LegacyModule(std::string const& module_label);

    ScheduleID scheduleID() const noexcept;

    using base_engine_t = EngineCreator::base_engine_t;
    using seed_t = EngineCreator::seed_t;
    using label_t = EngineCreator::label_t;

    using EngineCreator::createEngine;

    class ScheduleIDSentry;

  private:
    void setScheduleID(ScheduleID const sid) noexcept;

    // The thread-sanitizer wants this to be atomic, even though
    // it's unlikely to be a problem in any practical scenario.
    std::atomic<ScheduleID> scheduleID_;
  };

  class LegacyModule::ScheduleIDSentry {
  public:
    explicit ScheduleIDSentry(LegacyModule& mod, ScheduleID const sid) noexcept;
    ~ScheduleIDSentry() noexcept;

  private:
    LegacyModule& mod_;
  };
}

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_detail_LegacyModule_h */
