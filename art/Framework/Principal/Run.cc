#include "art/Framework/Principal/Run.h"

#include "art/Framework/Principal/RunPrincipal.h"
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

  bool
  Run::getProcessParameterSet(std::string const& /*processName*/,
                              std::vector<ParameterSet>& psets) const
  {
    // Get the relevant ProcessHistoryIDs
    auto get_history = [](ProcessHistoryID const& id) {
      ProcessHistory temp;
      ProcessHistoryRegistry::get(id, temp);
    };

    std::vector<ProcessHistoryID> historyIDs;
    cet::for_all( historyIDs, get_history );

    // Get the relevant ParameterSetIDs.
    auto fill_psets = [&psets](ParameterSetID const& id){
      ParameterSet temp;
      ParameterSetRegistry::get(id, temp);
      psets.push_back(temp);
    };

    std::vector<ParameterSetID> psetIdsUsed;
    cet::for_all( psetIdsUsed, fill_psets );

    return false;
  }

  void
  Run::commit_()
  {
    auto & rp = dynamic_cast<RunPrincipal &>(principal());
    auto put_in_principal = [&rp](auto& elem) {

      auto runProductProvenancePtr = std::make_unique<ProductProvenance const>(elem.first,
                                                                               productstatus::present());
      elem.second.prod->setRangeSetID(rp.aux().rangeSetID());
      rp.put( std::move(elem.second.prod),
              elem.second.bd,
              std::move(runProductProvenancePtr) );
    };

    cet::for_all( putProducts(), put_in_principal );

    // the cleanup is all or none
    putProducts().clear();
  }

}  // art
