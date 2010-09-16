//#include "art/Persistency/Common/RNGsnapshot.h"
//#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Provenance/BranchChildren.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/FileID.h"
#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Persistency/Provenance/Hash.h"
#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/ParameterSetBlob.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ParentageID.h"
#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/ProcessConfigurationID.h"
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "art/Persistency/Provenance/Transient.h"

#include "fhiclcpp/ParameterSetID.h"

#include <map>
#include <set>
#include <vector>

// These below are for backward compatibility only
// Note: ModuleDescription is still used, but is no longer persistent
#include "art/Persistency/Provenance/BranchEntryDescription.h"
#include "art/Persistency/Provenance/EntryDescription.h"
#include "art/Persistency/Provenance/EntryDescriptionID.h"
#include "art/Persistency/Provenance/EventAux.h"
#include "art/Persistency/Provenance/EventEntryDescription.h"
#include "art/Persistency/Provenance/EventEntryInfo.h"
#include "art/Persistency/Provenance/EventProcessHistoryID.h"
#include "art/Persistency/Provenance/SubRunAux.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ModuleDescriptionID.h"
#include "art/Persistency/Provenance/RunAux.h"
#include "art/Persistency/Provenance/RunSubRunEntryInfo.h"


namespace {
  struct dictionary {
  std::pair<edm::BranchKey, edm::BranchDescription> dummyPairBranch;
  std::map<fhicl::ParameterSetID, edm::ParameterSetBlob> dummyMapParam;
  std::map<edm::ProcessHistoryID, edm::ProcessHistory> dummyMapProcH;
  std::map<edm::ProcessConfigurationID, edm::ProcessConfiguration> dummyMapProcC;
  std::set<fhicl::ParameterSetID > dummySetParam;
  std::set<edm::ProcessHistoryID > dummySetProcH;
  std::set<edm::ProcessConfigurationID > dummySetProcessDesc;
  std::pair<fhicl::ParameterSetID, edm::ParameterSetBlob> dummyPairParam;
  std::pair<edm::ProcessHistoryID, edm::ProcessHistory> dummyPairProcH;
  std::pair<edm::ProcessConfigurationID, edm::ProcessConfiguration> dummyPairProcC;
  edm::ParentageID dummyParentageID;
  std::vector<edm::ProductID> dummyVectorProductID;
  std::vector<edm::BranchID> dummyVectorBranchID;
  std::set<edm::BranchID> dummySetBranchID;
  std::map<edm::BranchID, std::set<edm::BranchID> > dummyMapSetBranchID;
  std::pair<edm::BranchID, std::set<edm::BranchID> > dummyPairSetBranchID;
  std::vector<std::basic_string<char> > dummyVectorString;
  std::set<std::basic_string<char> > dummySetString;
  std::vector<edm::EventID> dummyVectorEventID;
  std::vector<std::vector<edm::EventID> > dummyVectorVectorEventID;
  std::vector<edm::ProductProvenance> dummyVectorProductProvenance;
  std::vector<std::vector<fhicl::ParameterSetID> > dummyVectorVectorParameterSetID;

//  edm::RNGsnapshot                             dummyRNGsnap;
//  std::vector<edm::RNGsnapshot>                dummyVectorRNGsnap;
//  edm::Wrapper<std::vector<edm::RNGsnapshot> > dummyWrapperVectorRNGsnap;

  // The remaining ones are for backward compatibility only.
  std::map<edm::ModuleDescriptionID, edm::ModuleDescription> dummyMapMod;
  std::pair<edm::ModuleDescriptionID, edm::ModuleDescription> dummyPairMod;
  std::vector<edm::EventProcessHistoryID> dummyEventProcessHistory;
  edm::EntryDescriptionID dummyEntryDescriptionID;
  std::vector<edm::EventEntryInfo> dummyVectorEventEntryInfo;
  std::vector<edm::RunSubRunEntryInfo> dummyVectorRunSubRunEntryInfo;
};
}
