#ifndef art_Utilities_CurrentProcessingContext_h
#define art_Utilities_CurrentProcessingContext_h
// vim: set sw=2 expandtab :

// CurrentProcessingContext is a class that carries information about
// the current event processing context. Each module in a framework
// job can access its CurrentProcessingContext *when that module is
// active in event processing*. At such a time, the
// CurrentProcessingContext will provide information about that
// module's place in the schedule, *as seen at that moment*.
//
// N.B.: An individual module instance can appear in more than one
// path; this is why CurrentProcessingContext reports the module's
// place in the schedule as seen at the time of execution. This is
// also why the module can not be queried for this information when
// it is not active in processing.

#include "art/Utilities/ScheduleID.h"

#include <string>

namespace art {

  class ModuleBase;
  class ModuleDescription;

  class CurrentProcessingContext {

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~CurrentProcessingContext() noexcept;

    CurrentProcessingContext() noexcept;

    explicit CurrentProcessingContext(ScheduleID scheduleID,
                                      std::string const* const name,
                                      int const bitpos,
                                      bool const isEndPth) noexcept;

    CurrentProcessingContext(CurrentProcessingContext const&) noexcept;

    CurrentProcessingContext(CurrentProcessingContext&&) noexcept;

    CurrentProcessingContext& operator=(
      CurrentProcessingContext const&) noexcept;

    CurrentProcessingContext& operator=(CurrentProcessingContext&&) noexcept;

  public: // MEMBER FUNCTIONS -- API for the user
    ScheduleID scheduleID() const noexcept;

    std::string const* pathName() const noexcept;

    int bitPos() const noexcept;

    bool isEndPath() const noexcept;

    int slotInPath() const noexcept;

    ModuleDescription const* moduleDescription() const noexcept;

    void activate(int theSlotInPath, ModuleDescription const*) noexcept;

  private:
    // What schedule this module is active on.
    ScheduleID scheduleID_{}; // invalid

    // Name of the currently active path.
    std::string const* pathName_{nullptr};

    // Index of the currently active path in the trigger results.
    int bitPos_{0};

    // Whether or not the currently active path is the end path.
    bool isEndPath_{false};

    // Index of the current active worker in the path.
    int slotInPath_{0};

    ModuleDescription const* moduleDescription_{nullptr};

    ModuleBase* module_{nullptr};
  };

} // namespace art

#endif /* art_Utilities_CurrentProcessingContext_h */

// Local Variables:
// mode: c++
// End:
