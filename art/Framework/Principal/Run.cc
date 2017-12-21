#include "art/Framework/Principal/Run.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/RunPrincipal.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"

using fhicl::ParameterSet;
using fhicl::ParameterSetID;
using fhicl::ParameterSetRegistry;

namespace art {

  Run::~Run() {}

  Run::Run(RunPrincipal const& rp,
           ModuleDescription const& md,
           TypeLabelLookup_t const& expectedProducts,
           RangeSet const& rs /*= RangeSet::invalid()*/)
    : DataViewImpl{InRun, rp, md, false, expectedProducts, rs}
  {}

  RunID
  Run::id() const
  {
    return DataViewImpl::runID();
  }

} // namespace art
