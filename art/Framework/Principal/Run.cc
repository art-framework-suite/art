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

Run::
~Run()
{
}

Run::
Run(RunPrincipal const& rp, ModuleDescription const& md, RangeSet const& rs /*= RangeSet::invalid()*/)
  : DataViewImpl{InRun, rp, md, false, rs}
{
}

RunID const
Run::
id() const
{
  return DataViewImpl::runID();
}

} // namespace art
