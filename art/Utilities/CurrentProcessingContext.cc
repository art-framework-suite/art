#include "art/Utilities/CurrentProcessingContext.h"
// vim: set sw=2 expandtab :

#include "art/Utilities/ScheduleID.h"

#include <atomic>
#include <string>
#include <utility>

using namespace std;

namespace art {

  class ModuleDescription;

  CurrentProcessingContext::~CurrentProcessingContext() noexcept
  {
    pathName_ = nullptr;
    moduleDescription_ = nullptr;
    module_ = nullptr;
  }

  CurrentProcessingContext::CurrentProcessingContext() noexcept
  {
    scheduleID_ = ScheduleID::first();
    pathName_ = nullptr;
    bitPos_ = 0;
    isEndPath_ = false;
    slotInPath_ = 0;
    moduleDescription_ = nullptr;
    module_ = nullptr;
  }

  CurrentProcessingContext::CurrentProcessingContext(
    ScheduleID const si,
    string const* const name,
    int const bitpos,
    bool const isEndPath) noexcept
  {
    scheduleID_ = si;
    pathName_ = name;
    bitPos_ = bitpos;
    isEndPath_ = isEndPath;
    slotInPath_ = 0;
    moduleDescription_ = nullptr;
    module_ = nullptr;
  }

  CurrentProcessingContext::CurrentProcessingContext(
    CurrentProcessingContext const& rhs) noexcept
  {
    scheduleID_ = rhs.scheduleID_.load();
    pathName_ = rhs.pathName_.load();
    bitPos_ = rhs.bitPos_.load();
    isEndPath_ = rhs.isEndPath_.load();
    slotInPath_ = rhs.slotInPath_.load();
    moduleDescription_ = rhs.moduleDescription_.load();
    module_ = rhs.module_.load();
  }

  CurrentProcessingContext::CurrentProcessingContext(
    CurrentProcessingContext&& rhs) noexcept
  {
    scheduleID_ = rhs.scheduleID_.load();
    pathName_ = rhs.pathName_.load();
    bitPos_ = rhs.bitPos_.load();
    isEndPath_ = rhs.isEndPath_.load();
    slotInPath_ = rhs.slotInPath_.load();
    moduleDescription_ = rhs.moduleDescription_.load();
    module_ = rhs.module_.load();
  }

  CurrentProcessingContext&
  CurrentProcessingContext::operator=(
    CurrentProcessingContext const& rhs) noexcept
  {
    if (this != &rhs) {
      scheduleID_ = rhs.scheduleID_.load();
      pathName_ = rhs.pathName_.load();
      bitPos_ = rhs.bitPos_.load();
      isEndPath_ = rhs.isEndPath_.load();
      slotInPath_ = rhs.slotInPath_.load();
      moduleDescription_ = rhs.moduleDescription_.load();
      module_ = rhs.module_.load();
    }
    return *this;
  }

  CurrentProcessingContext&
  CurrentProcessingContext::operator=(CurrentProcessingContext&& rhs) noexcept
  {
    scheduleID_ = rhs.scheduleID_.load();
    pathName_ = rhs.pathName_.load();
    bitPos_ = rhs.bitPos_.load();
    isEndPath_ = rhs.isEndPath_.load();
    slotInPath_ = rhs.slotInPath_.load();
    moduleDescription_ = rhs.moduleDescription_.load();
    module_ = rhs.module_.load();
    return *this;
  }

  ScheduleID
  CurrentProcessingContext::scheduleID() const noexcept
  {
    return scheduleID_.load();
  }

  string const*
  CurrentProcessingContext::pathName() const noexcept
  {
    return pathName_.load();
  }

  int
  CurrentProcessingContext::bitPos() const noexcept
  {
    return bitPos_.load();
  }

  bool
  CurrentProcessingContext::isEndPath() const noexcept
  {
    return isEndPath_.load();
  }

  int
  CurrentProcessingContext::slotInPath() const noexcept
  {
    return slotInPath_.load();
  }

  ModuleDescription const*
  CurrentProcessingContext::moduleDescription() const noexcept
  {
    return moduleDescription_.load();
  }

  void
  CurrentProcessingContext::activate(int theSlotInPath,
                                     ModuleDescription const* md) noexcept
  {
    slotInPath_ = theSlotInPath;
    moduleDescription_ = md;
  }

} // namespace art
