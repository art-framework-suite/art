// ======================================================================
//
// ProvenanceCheckerOutput: Check the consistency of provenance stored
//                          in the framework
//
// ======================================================================

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Utilities/ConfigurationTable.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "cetlib/exception.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace art {

  class ProvenanceCheckerOutput : public OutputModule {
  public:

    struct Config {
      fhicl::TableFragment<OutputModule::Config> omConfig;
    };

    using Parameters = art::WrappedTable<Config, OutputModule::Config::KeysToIgnore>;
    explicit ProvenanceCheckerOutput(Parameters const&);

  private:
    void write(EventPrincipal& e) override;
    void writeSubRun(SubRunPrincipal&) override {}
    void writeRun(RunPrincipal&) override {}
  };  // ProvenanceCheckerOutput

  //
  // constructors and destructor
  //
  ProvenanceCheckerOutput::ProvenanceCheckerOutput(ProvenanceCheckerOutput::Parameters const& ps) :
    OutputModule{ps().omConfig, ps.get_PSet()}
  { }

  //
  // member functions
  //
  static void markAncestors(const ProductProvenance& iInfo,
                            const BranchMapper& iMapper,
                            std::map<BranchID,bool>& oMap,
                            std::set<BranchID>& oMapperMissing) {
    for (auto const& parent : iInfo.parentage().parents()) {
      //Don't look for parents if we've previously looked at the parents
      if (oMap.find(parent) == oMap.end()) {
        //use side effect of calling operator[] which is if the item isn't there it will add it as 'false'
        oMap[parent];
        cet::exempt_ptr<ProductProvenance const> pInfo = iMapper.branchToProductProvenance(parent);
        if (pInfo.get()) {
          markAncestors(*pInfo,iMapper,oMap,oMapperMissing);
        } else {
          oMapperMissing.insert(parent);
        }
      }
    }
  }

  void
  ProvenanceCheckerOutput::write(EventPrincipal& e)
  {
    //check ProductProvenance's parents to see if they are in the ProductProvenance list
    BranchMapper const& mapper = const_cast<EventPrincipal const&>(e).branchMapper();

    std::map<BranchID,bool> seenParentInPrincipal;
    std::set<BranchID> missingFromMapper;
    std::set<BranchID> missingProductProvenance;

    for (auto const& group : e) {
      if (group.second && !group.second->productUnavailable()) {
        //This call seems to have a side effect of filling the 'ProductProvenance' in the Group
        e.getForOutput(group.first, false);

        if (not group.second->productProvenancePtr().get() ) {
          missingProductProvenance.insert(group.first);
          continue;
        }
        cet::exempt_ptr<ProductProvenance const> pInfo = mapper.branchToProductProvenance(group.first);
        if (!pInfo.get()) {
          missingFromMapper.insert(group.first);
        }
        markAncestors(*(group.second->productProvenancePtr()),mapper,seenParentInPrincipal, missingFromMapper);
      }
      seenParentInPrincipal[group.first]=true;
    }

    //Determine what BranchIDs are in the product registry
    auto const& prodList = ProductMetaData::instance().productList();
    std::set<BranchID> branchesInReg;
    for (auto const& prod : prodList) {
      branchesInReg.insert(prod.second.branchID());
    }

    std::set<BranchID> missingFromPrincipal;
    std::set<BranchID> missingFromReg;
    for (auto const& seenParent : seenParentInPrincipal) {
      if (!seenParent.second) {
        missingFromPrincipal.insert(seenParent.first);
      }
      if (branchesInReg.find(seenParent.first) == branchesInReg.end()) {
        missingFromReg.insert(seenParent.first);
      }
    }

    auto logBranchID = [](auto const& missing){ mf::LogProblem("ProvenanceChecker") << missing; };

    if (missingFromMapper.size()) {
      mf::LogError("ProvenanceChecker") << "Missing the following BranchIDs from BranchMapper\n";
      cet::for_all(missingFromMapper, logBranchID);
    }

    if (missingFromPrincipal.size()) {
      mf::LogError("ProvenanceChecker") << "Missing the following BranchIDs from EventPrincipal\n";
      cet::for_all(missingFromPrincipal, logBranchID);
    }

    if (missingProductProvenance.size()) {
      mf::LogError("ProvenanceChecker") << "The Groups for the following BranchIDs have no ProductProvenance\n";
      cet::for_all(missingProductProvenance, logBranchID);
    }

    if (missingFromReg.size()) {
      mf::LogError("ProvenanceChecker") << "Missing the following BranchIDs from ProductRegistry\n";
      cet::for_all(missingFromReg, logBranchID);
    }

    if (missingFromMapper.size() or missingFromPrincipal.size() or missingProductProvenance.size() or missingFromReg.size()) {
      throw cet::exception("ProvenanceError")
        << (missingFromMapper.size() or missingFromPrincipal.size()?"Having missing ancestors": "")
        << (missingFromMapper.size()?" from BranchMapper":"")
        << (missingFromMapper.size() and missingFromPrincipal.size()?" and":"")
        << (missingFromPrincipal.size()?" from EventPrincipal":"")
        << (missingFromMapper.size() or missingFromPrincipal.size()?".\n":"")
        << (missingProductProvenance.size()?" Have missing ProductProvenance's from Group in EventPrincipal.\n":"")
        << (missingFromReg.size()?" Have missing info from ProductRegistry.\n":"");
    }
  }

}  // art

// ======================================================================

DEFINE_ART_MODULE(art::ProvenanceCheckerOutput)

// ======================================================================
