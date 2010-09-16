#ifndef IOPool_Input_ProvenanceAdaptor_h
#define IOPool_Input_ProvenanceAdaptor_h

/*----------------------------------------------------------------------

ProvenanceAdaptor.h

----------------------------------------------------------------------*/


#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Persistency/Provenance/ModuleDescriptionRegistry.h"
#include "art/Persistency/Provenance/ParameterSetBlob.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"

#include "fhiclcpp/ParameterSetID.h"

#include "boost/shared_ptr.hpp"
#include "boost/utility.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>


namespace edm {

  //------------------------------------------------------------
  // Class ProvenanceAdaptor: supports file reading.

  typedef std::map<fhicl::ParameterSetID, ParameterSetBlob> ParameterSetMap;
  class ProvenanceAdaptor : private boost::noncopyable {
  public:
  ProvenanceAdaptor(
             ProductRegistry const& productRegistry,
             ProcessHistoryMap const& pHistMap,
             ParameterSetMap const& psetMap,
             ModuleDescriptionMap const&  mdMap);


  boost::shared_ptr<BranchIDLists const> branchIDLists() const;

  void branchListIndexes(BranchListIndexes & indexes) const;

  private:
    ProductRegistry const& productRegistry_;
    boost::shared_ptr<BranchIDLists const> branchIDLists_;
    std::vector<BranchListIndex> branchListIndexes_;
  }; // class ProvenanceAdaptor

}  // namespace edm

#endif  // IOPool_Input_ProvenanceAdaptor_h
