#include "art/Utilities/CurrentProcessingContext.h"
// vim: set sw=2 expandtab :

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

  CurrentProcessingContext::CurrentProcessingContext() noexcept = default;

  CurrentProcessingContext::CurrentProcessingContext(
    ScheduleID const si,
    string const* const name,
    int const bitpos,
    bool const isEndPth) noexcept
    : scheduleID_{si}, pathName_{name}, bitPos_{bitpos}, isEndPath_{isEndPth}
  {}

  CurrentProcessingContext::CurrentProcessingContext(
    CurrentProcessingContext const& rhs) noexcept = default;

  CurrentProcessingContext::CurrentProcessingContext(
    CurrentProcessingContext&& rhs) noexcept = default;
  CurrentProcessingContext& CurrentProcessingContext::operator=(
    CurrentProcessingContext const& rhs) noexcept = default;

  CurrentProcessingContext& CurrentProcessingContext::operator=(
    CurrentProcessingContext&& rhs) noexcept = default;

  ScheduleID
  CurrentProcessingContext::scheduleID() const noexcept
  {
    return scheduleID_;
  }

  string const*
  CurrentProcessingContext::pathName() const noexcept
  {
    return pathName_;
  }

  int
  CurrentProcessingContext::bitPos() const noexcept
  {
    return bitPos_;
  }

  bool
  CurrentProcessingContext::isEndPath() const noexcept
  {
    return isEndPath_;
  }

  int
  CurrentProcessingContext::slotInPath() const noexcept
  {
    return slotInPath_;
  }

  ModuleDescription const*
  CurrentProcessingContext::moduleDescription() const noexcept
  {
    return moduleDescription_;
  }

  void
  CurrentProcessingContext::activate(int theSlotInPath,
                                     ModuleDescription const* md) noexcept
  {
    slotInPath_ = theSlotInPath;
    moduleDescription_ = md;
  }

} // namespace art
