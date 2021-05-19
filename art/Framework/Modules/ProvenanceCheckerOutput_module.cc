// ======================================================================
//
// ProvenanceCheckerOutput: Check the consistency of provenance stored
//                          in the framework
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "canvas/Persistency/Provenance/Parentage.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace art {

  class ProvenanceCheckerOutput : public OutputModule {
  public:
    struct Config {
      fhicl::TableFragment<OutputModule::Config> omConfig;
    };

    using Parameters =
      fhicl::WrappedTable<Config, OutputModule::Config::KeysToIgnore>;
    explicit ProvenanceCheckerOutput(Parameters const&);

  private:
    void write(EventPrincipal& e) override;
    void
    writeSubRun(SubRunPrincipal&) override
    {}
    void
    writeRun(RunPrincipal&) override
    {}
  }; // ProvenanceCheckerOutput

  //
  // constructors and destructor
  //
  ProvenanceCheckerOutput::ProvenanceCheckerOutput(
    ProvenanceCheckerOutput::Parameters const& ps)
    : OutputModule{ps().omConfig, ps.get_PSet()}
  {}

  //
  // member functions
  //
  static void
  markAncestors(ProductProvenance const& iInfo,
                EventPrincipal& e,
                std::map<ProductID, bool>& oMap,
                std::set<ProductID>& oMapperMissing)
  {
    for (art::ProductID const parent : iInfo.parentage().parents()) {
      // Don't look for parents if we've previously looked at the parents
      if (oMap.find(parent) == oMap.end()) {
        // use side effect of calling operator[] which is if the item isn't
        // there it will add it as 'false'
        oMap[parent];
        cet::exempt_ptr<ProductProvenance const> pInfo =
          e.branchToProductProvenance(parent);
        if (pInfo.get()) {
          markAncestors(*pInfo, e, oMap, oMapperMissing);
        } else {
          oMapperMissing.insert(parent);
        }
      }
    }
  }

  void
  ProvenanceCheckerOutput::write(EventPrincipal& e)
  {
    // Check ProductProvenance's parents to see if they are in the
    // ProductProvenance list

    std::map<ProductID, bool> seenParentInPrincipal;
    std::set<ProductID> missingFromMapper;
    std::set<ProductID> missingProductProvenance;

    for (auto const& group : e) {
      auto const pid = group.first;
      auto const& pd = group.second;
      if (pd && pd->productAvailable()) {
        e.getForOutput(pid, false);
        if (not pd->productProvenance().get()) {
          missingProductProvenance.insert(pid);
          continue;
        }
        auto pInfo = e.branchToProductProvenance(pid);
        if (!pInfo) {
          missingFromMapper.insert(pid);
        }
        markAncestors(*(pd->productProvenance()),
                      e,
                      seenParentInPrincipal,
                      missingFromMapper);
      }
      seenParentInPrincipal[pid] = true;
    }

    // Determine what ProductIDs are missing from the principal,
    // vs. which ProductIDs are not even accessible to the principal
    // via the product tables.
    std::set<ProductID> missingFromPrincipal;
    std::set<ProductID> missingFromTables;
    for (auto const& seenParent : seenParentInPrincipal) {
      if (!seenParent.second) {
        missingFromPrincipal.insert(seenParent.first);
      }
      ProductID const pid{seenParent.first};
      auto found = e.getProductDescription(pid);
      if (found == nullptr) {
        missingFromTables.insert(pid);
      }
    }

    auto logProductID = [](auto const& missing) {
      mf::LogProblem("ProvenanceChecker") << missing;
    };

    if (missingFromMapper.size()) {
      mf::LogError("ProvenanceChecker")
        << "Missing the following ProductIDs from BranchMapper\n";
      cet::for_all(missingFromMapper, logProductID);
    }

    if (missingFromPrincipal.size()) {
      mf::LogError("ProvenanceChecker")
        << "Missing the following ProductIDs from EventPrincipal\n";
      cet::for_all(missingFromPrincipal, logProductID);
    }

    if (missingProductProvenance.size()) {
      mf::LogError("ProvenanceChecker") << "The Groups for the following "
                                           "ProductIDs have no "
                                           "ProductProvenance\n";
      cet::for_all(missingProductProvenance, logProductID);
    }

    if (missingFromTables.size()) {
      mf::LogError("ProvenanceChecker") << "Missing the following ProductIDs "
                                           "from the principal's product "
                                           "tables\n";
      cet::for_all(missingFromTables, logProductID);
    }

    if (missingFromMapper.size() or missingFromPrincipal.size() or
        missingProductProvenance.size() or missingFromTables.size()) {
      throw cet::exception("ProvenanceError")
        << (missingFromMapper.size() or missingFromPrincipal.size() ?
              "Having missing ancestors" :
              "")
        << (missingFromMapper.size() ? " from BranchMapper" : "")
        << (missingFromMapper.size() and missingFromPrincipal.size() ? " and" :
                                                                       "")
        << (missingFromPrincipal.size() ? " from EventPrincipal" : "")
        << (missingFromMapper.size() or missingFromPrincipal.size() ? ".\n" :
                                                                      "")
        << (missingProductProvenance.size() ? " Have missing "
                                              "ProductProvenance's from Group "
                                              "in EventPrincipal.\n" :
                                              "")
        << (missingFromTables.size() ?
              " Have missing info from the principal's product tables.\n" :
              "");
    }
  }

} // namespace art

// ======================================================================

DEFINE_ART_MODULE(art::ProvenanceCheckerOutput)

// ======================================================================
