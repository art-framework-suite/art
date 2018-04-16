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

#include <atomic>
#include <string>

namespace art {

  class ModuleBase;
  class ModuleDescription;

  class CurrentProcessingContext {
  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~CurrentProcessingContext() noexcept;
    CurrentProcessingContext() noexcept;
    explicit CurrentProcessingContext(ScheduleID const si,
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

  public: // MEMBER FUNCTIONS -- API for Path to set the WorkerInPath and module
    void activate(int theSlotInPath, ModuleDescription const*) noexcept;

  private: // MEMBER DATA -- Stream info
    // What schedule this module is active on.
    std::atomic<ScheduleID> scheduleID_;

  private: // MEMBER DATA -- Path info
    // Name of the currently active path.
    std::atomic<std::string const*> pathName_;
    // Index of the currently active
    // path in the trigger results.
    std::atomic<int> bitPos_;
    // Whether or not the currently
    // active path is the end path.
    std::atomic<bool> isEndPath_;

  private: // MEMBER DATA -- WorkerInPath info
    // Index of the current active worker in the path.
    std::atomic<int> slotInPath_;

  private: // MEMBER DATA -- Module info
    std::atomic<ModuleDescription const*> moduleDescription_;
    std::atomic<ModuleBase*> module_;
  };

} // namespace art

#endif /* art_Utilities_CurrentProcessingContext_h */

// Local Variables:
// mode: c++
// End:
