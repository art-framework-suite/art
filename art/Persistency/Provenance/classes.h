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

template class std::vector<art::ProductProvenance>;
//template class art::ProcessHistory::collection_type;
template class std::vector<art::ProcessConfiguration>;
template class std::vector<art::ProcessHistory>;
template class std::set<art::BranchID>;
template class std::pair<art::BranchID, std::set<art::BranchID>>;
template class std::map<art::BranchID, std::set<art::BranchID>>;
template class std::pair<art::BranchKey, art::BranchDescription>;
template class std::map<art::BranchKey, art::BranchDescription>;
template class std::set<fhicl::ParameterSetID>;
//template class art::ProcessConfigurationID;
//template class art::Hash<art::ProcessConfigurationType>;
template class std::set<art::ProcessConfigurationID>;
template class std::vector<art::BranchID>;
template class std::set<art::ProcessHistoryID>;
template class std::vector<art::FileIndex::Element>;
//template class art::ProcessHistoryMap;
template class std::map<art::ProcessHistoryID const, art::ProcessHistory>;
//template class std::pair<const art::ProcessHistoryID, art::ProcessHistory>;
//template class art::ParameterSetMap;
template class std::map<fhicl::ParameterSetID, art::ParameterSetBlob>;
template class std::pair<fhicl::ParameterSetID, art::ParameterSetBlob>;

