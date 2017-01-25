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

  Run::Run(RunPrincipal const& rp, ModuleDescription const& md, RangeSet const& rs) :
    DataViewImpl{rp, md, InRun},
    aux_{rp.aux()},
    productRangeSet_{rs}
  {}

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
  Run::commit_(RunPrincipal& rp)
  {
    auto put_in_principal = [&rp](auto& elem) {

      auto runProductProvenancePtr = std::make_unique<ProductProvenance const>(elem.first,
                                                                               productstatus::present());
      rp.put( std::move(elem.second.prod),
              elem.second.bd,
              std::move(runProductProvenancePtr),
              std::move(elem.second.rs) );
    };

    cet::for_all( putProducts(), put_in_principal );

    // the cleanup is all or none
    putProducts().clear();
  }

}  // art
