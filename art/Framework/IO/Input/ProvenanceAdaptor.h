#ifndef IORoot_Input_ProvenanceAdaptor_h
#define IORoot_Input_ProvenanceAdaptor_h

// ======================================================================
//
// ProvenanceAdaptor - supports file reading.
//
// ======================================================================


#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Persistency/Provenance/ModuleDescriptionRegistry.h"
#include "art/Persistency/Provenance/ParameterSetBlob.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "fhiclcpp/ParameterSetID.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

// ----------------------------------------------------------------------

namespace art {

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
  }; // ProvenanceAdaptor

}  // art

// ======================================================================

#endif
