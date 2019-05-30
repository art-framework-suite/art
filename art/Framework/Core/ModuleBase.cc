#include "art/Framework/Core/ModuleBase.h"
// vim: set sw=2 expandtab :

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"

using namespace std;

namespace art {

  ModuleBase::~ModuleBase() noexcept = default;
  ModuleBase::ModuleBase() = default;

  ModuleDescription const&
  ModuleBase::moduleDescription() const
  {
    if (md_.has_value()) {
      return *md_;
    }

    throw Exception{errors::LogicError,
                    "There was an error while calling moduleDescription().\n"}
      << "The moduleDescription() base-class member function cannot be called\n"
         "during module construction.  To determine which module is "
         "responsible\n"
         "for calling it, find the '<module type>:<module "
         "label>@Construction'\n"
         "tag in the message prefix above.  Please contact artists@fnal.gov\n"
         "for guidance.\n";
  }

  void
  ModuleBase::setModuleDescription(ModuleDescription const& md)
  {
    md_ = md;
  }

  array<vector<ProductInfo>, NumBranchTypes> const&
  ModuleBase::getConsumables() const
  {
    return collector_.getConsumables();
  }

  void
  ModuleBase::sortConsumables(std::string const& current_process_name)
  {
    // Now that we know we have seen all the consumes declarations,
    // sort the results for fast lookup later.
    collector_.sortConsumables(current_process_name);
  }

  ConsumesCollector&
  ModuleBase::consumesCollector()
  {
    return collector_;
  }

} // namespace art
