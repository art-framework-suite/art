#include "art/Framework/Principal/CurrentProcessingContext.h"

#include "art/Persistency/Provenance/ModuleDescription.h"
#include <cassert>

namespace art {
  CurrentProcessingContext::CurrentProcessingContext() :
    pathInSchedule_(0),
    slotInPath_(0),
    moduleDescription_(0),
    pathName_(0),
    isEndPath_(false)
  { }

  CurrentProcessingContext::CurrentProcessingContext(std::string const* name,
                                                     int bitpos,
                                                     bool isEndPth) :
    pathInSchedule_(bitpos),
    slotInPath_(0),
    moduleDescription_(0),
    pathName_(name),
    isEndPath_(isEndPth)
  { }

  std::string const*
  CurrentProcessingContext::moduleLabel() const
  {
    return is_active()
      ? &(moduleDescription_->moduleLabel())
      : 0;
  }

  std::string const*
  CurrentProcessingContext::pathName() const
  {
    return pathName_;
  }

  ModuleDescription const*
  CurrentProcessingContext::moduleDescription() const
  {
    return moduleDescription_;
  }

  int
  CurrentProcessingContext::pathInSchedule() const
  {
    return is_active()
      ? pathInSchedule_
      : -1;
  }


  int
  CurrentProcessingContext::slotInPath() const
  {
    return is_active()
      ? static_cast<int>(slotInPath_)
      : -1;
  }

  bool
  CurrentProcessingContext::isEndPath() const
  {
    return isEndPath_;
  }

  void
  CurrentProcessingContext::activate(size_t theSlotInPath,
                                     ModuleDescription const* mod)
  {
    assert( mod );
    slotInPath_     = theSlotInPath;
    moduleDescription_ = mod;
  }

  void
  CurrentProcessingContext::deactivate()
  {
    pathInSchedule_    = 0;
    slotInPath_        = 0;
    moduleDescription_ = 0;
    pathName_          = 0;
  }

}  // art
