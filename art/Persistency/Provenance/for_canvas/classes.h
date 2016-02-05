#include "canvas/Persistency/Provenance/BranchChildren.h"
#include "canvas/Persistency/Provenance/BranchID.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ParameterSetMap.h"
#include "canvas/Persistency/Provenance/Parentage.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Persistency/Provenance/ProcessConfigurationID.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductRegistry.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/ResultsAuxiliary.h"

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
