#include "art/Persistency/Provenance/BranchChildren.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/ParameterSetMap.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/ProcessConfigurationID.h"
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/ResultsAuxiliary.h"

namespace {
   struct dictionary {
      std::vector<art::ProductProvenance> d1;
      std::vector<art::ProcessConfiguration> d2;
      std::vector<art::ProcessHistory> d3;
      art::ProcessHistory::collection_type d4;
      std::set<art::BranchID> d5;
      std::pair<art::BranchID, std::set<art::BranchID> > d6;
      std::map<art::BranchID, std::set<art::BranchID> > d7;
      art::BranchKey d8;
      art::BranchDescription d9;
      std::pair<art::BranchKey, art::BranchDescription> d10;
      std::map<art::BranchKey, art::BranchDescription> d11;
      std::set<fhicl::ParameterSetID> d12;
      art::ProcessConfigurationID d13a;
      std::set<art::ProcessConfigurationID> d13;
      std::vector<art::BranchID> d14;
      std::set<art::ProcessHistoryID> d15;
      std::vector<art::FileIndex::Element> d16;
      art::ProcessHistoryMap d17;
      std::pair<const art::ProcessHistoryID, art::ProcessHistory> d18;
      art::ParameterSetMap d19;
      std::pair<fhicl::ParameterSetID, art::ParameterSetBlob> d20;
   };
}
