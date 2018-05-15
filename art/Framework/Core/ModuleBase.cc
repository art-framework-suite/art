#include "art/Framework/Core/ModuleBase.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductToken.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace CLHEP {
  class HepRandomEngine;
}

using namespace hep::concurrency;
using namespace std;

namespace art {

  ModuleBase::~ModuleBase() noexcept = default;
  ModuleBase::ModuleBase() = default;

  ModuleDescription const&
  ModuleBase::moduleDescription() const
  {
    return md_;
  }

  void
  ModuleBase::setModuleDescription(ModuleDescription const& md)
  {
    md_ = md;
  }

  array<vector<ProductInfo>, NumBranchTypes> const&
  ModuleBase::getConsumables() const
  {
    return consumables_;
  }

  void
  ModuleBase::sortConsumables(std::string const& current_process_name)
  {
    // Now that we know we have seen all the consumes declarations,
    // sort the results for fast lookup later.
    for (auto& vecPI : consumables_) {
      for (auto& pi : vecPI) {
        pi.process_ = ProcessTag{pi.process_.name(), current_process_name};
      }
      sort(vecPI.begin(), vecPI.end());
    }
  }

} // namespace art
