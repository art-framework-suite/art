#include "art/Framework/Core/Run.h"

#include "art/Framework/Core/RunPrincipal.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include <vector>


using fhicl::ParameterSet;
using fhicl::ParameterSetID;
using fhicl::ParameterSetRegistry;


namespace art {

  Run::Run(RunPrincipal& rp, ModuleDescription const& md) :
        DataViewImpl(rp, md, InRun),
        aux_(rp.aux()) {
  }

  RunPrincipal &
  Run::runPrincipal() {
    return dynamic_cast<RunPrincipal &>(principal());
  }

  RunPrincipal const &
  Run::runPrincipal() const {
    return dynamic_cast<RunPrincipal const&>(principal());
  }

  bool
  Run::getProcessParameterSet(std::string const& processName,
                              std::vector<ParameterSet>& psets) const
  {
    // Get the relevant ProcessHistoryIDs
    std::vector<ProcessHistoryID> historyIDs;

    // Get the relevant ParameterSetIDs.
    std::vector<ParameterSetID> psetIdsUsed;
    for (std::vector<ProcessHistoryID>::const_iterator
           i = historyIDs.begin(), e = historyIDs.end();
         i != e;
         ++i)
      {
        ProcessHistory temp;
        ProcessHistoryRegistry::get(*i, temp);
      }

    // Look up the ParameterSets for these IDs.
    for (std::vector<ParameterSetID>::const_iterator
           i = psetIdsUsed.begin(), e = psetIdsUsed.end();
         i != e;
         ++i)
      {
        ParameterSet temp;
        ParameterSetRegistry::get(*i, temp);
        psets.push_back(temp);
      }

    return false;
  }

  void
  Run::commit_() {
    // fill in guts of provenance here
    RunPrincipal & rp = runPrincipal();
    ProductPtrVec::iterator pit(putProducts().begin());
    ProductPtrVec::iterator pie(putProducts().end());

    while(pit!=pie) {
        std::auto_ptr<EDProduct> pr(pit->first);
        // note: ownership has been passed - so clear the pointer!
        pit->first = 0;

        // set provenance
        std::auto_ptr<ProductProvenance> runEntryInfoPtr(
                new ProductProvenance(pit->second->branchID(),
                                    productstatus::present()));
        rp.put(pr, *pit->second, runEntryInfoPtr);
        ++pit;
    }

    // the cleanup is all or none
    putProducts().clear();
  }

}  // art
