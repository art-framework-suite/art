#include "art/Utilities/CurrentProcessingContext.h"
// vim: set sw=2 expandtab :

#include <string>
#include <utility>

using namespace std;

namespace art {

class ModuleDescription;

CurrentProcessingContext::
~CurrentProcessingContext() noexcept
{
  pathName_ = nullptr;
  moduleDescription_ = nullptr;
  module_ = nullptr;
}

CurrentProcessingContext::
CurrentProcessingContext() noexcept
  : streamIndex_{0}
  , pathName_{nullptr}
  , bitPos_{0}
  , isEndPath_{false}
  , slotInPath_{0}
  , moduleDescription_{nullptr}
  , module_{nullptr}
{
}

CurrentProcessingContext::
CurrentProcessingContext(int si, string const* const name, int const bitpos, bool const isEndPth) noexcept
  : streamIndex_{si}
  , pathName_{name}
  , bitPos_{bitpos}
  , isEndPath_{isEndPth}
  , slotInPath_{0}
  , moduleDescription_{nullptr}
  , module_{nullptr}
{
}

CurrentProcessingContext::
CurrentProcessingContext(CurrentProcessingContext const& rhs) noexcept
  : streamIndex_{rhs.streamIndex_}
  , pathName_{rhs.pathName_}
  , bitPos_{rhs.bitPos_}
  , isEndPath_{rhs.isEndPath_}
  , slotInPath_{rhs.slotInPath_}
  , moduleDescription_{rhs.moduleDescription_}
  , module_{rhs.module_}
{
}

CurrentProcessingContext::
CurrentProcessingContext(CurrentProcessingContext&& rhs) noexcept
  : streamIndex_{move(rhs.streamIndex_)}
  , pathName_{move(rhs.pathName_)}
  , bitPos_{move(rhs.bitPos_)}
  , isEndPath_{move(rhs.isEndPath_)}
  , slotInPath_{move(rhs.slotInPath_)}
  , moduleDescription_{move(rhs.moduleDescription_)}
  , module_{move(rhs.module_)}
{
}

CurrentProcessingContext&
CurrentProcessingContext::
operator=(CurrentProcessingContext const& rhs) noexcept
{
  if (this != &rhs) {
    streamIndex_ = rhs.streamIndex_;
    pathName_ = rhs.pathName_;
    bitPos_ = rhs.bitPos_;
    isEndPath_ = rhs.isEndPath_;
    slotInPath_ = rhs.slotInPath_;
    moduleDescription_ = rhs.moduleDescription_;
    module_ = rhs.module_;
  }
  return *this;
}

CurrentProcessingContext&
CurrentProcessingContext::
operator=(CurrentProcessingContext&& rhs) noexcept
{
  streamIndex_ = move(rhs.streamIndex_);
  pathName_ = move(rhs.pathName_);
  bitPos_ = move(rhs.bitPos_);
  isEndPath_ = move(rhs.isEndPath_);
  slotInPath_ = move(rhs.slotInPath_);
  moduleDescription_ = move(rhs.moduleDescription_);
  module_ = move(rhs.module_);
  return *this;
}

int
CurrentProcessingContext::
streamIndex() const noexcept
{
  return streamIndex_;
}

string const*
CurrentProcessingContext::
pathName() const noexcept
{
  return pathName_;
}

int
CurrentProcessingContext::
bitPos() const noexcept
{
  return bitPos_;
}

bool
CurrentProcessingContext::
isEndPath() const noexcept
{
  return isEndPath_;
}

int
CurrentProcessingContext::
slotInPath() const noexcept
{
  return slotInPath_;
}

ModuleDescription const*
CurrentProcessingContext::
moduleDescription() const noexcept
{
  return moduleDescription_;
}

//ModuleBase*
//CurrentProcessingContext::
//module() const noexcept
//{
//  return module_;
//}

void
CurrentProcessingContext::
activate(int theSlotInPath, ModuleDescription const* md) noexcept
{
  slotInPath_ = theSlotInPath;
  moduleDescription_ = md;
}

//void
//CurrentProcessingContext::
//setModule(ModuleBase* module) noexcept
//{
//  module_ = module;
//}

} // namespace art

