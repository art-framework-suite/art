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
  std::pair<art::BranchKey, art::BranchDescription> dummyPairBranch;
  std::map<fhicl::ParameterSetID, art::ParameterSetBlob> dummyMapParam;
  std::map<art::ProcessHistoryID, art::ProcessHistory> dummyMapProcH;
  std::map<art::ProcessConfigurationID, art::ProcessConfiguration> dummyMapProcC;
  std::set<fhicl::ParameterSetID > dummySetParam;
  std::set<art::ProcessHistoryID > dummySetProcH;
  std::set<art::ProcessConfigurationID > dummySetProcessDesc;
  std::pair<fhicl::ParameterSetID, art::ParameterSetBlob> dummyPairParam;
  std::pair<art::ProcessHistoryID, art::ProcessHistory> dummyPairProcH;
  std::pair<art::ProcessConfigurationID, art::ProcessConfiguration> dummyPairProcC;
  art::ParentageID dummyParentageID;
  std::vector<art::ProductID> dummyVectorProductID;
  std::vector<art::BranchID> dummyVectorBranchID;
  std::set<art::BranchID> dummySetBranchID;
  std::map<art::BranchID, std::set<art::BranchID> > dummyMapSetBranchID;
  std::pair<art::BranchID, std::set<art::BranchID> > dummyPairSetBranchID;
  std::vector<std::basic_string<char> > dummyVectorString;
  std::set<std::basic_string<char> > dummySetString;
  std::vector<art::EventID> dummyVectorEventID;
  std::vector<std::vector<art::EventID> > dummyVectorVectorEventID;
  std::vector<art::ProductProvenance> dummyVectorProductProvenance;
  std::vector<std::vector<fhicl::ParameterSetID> > dummyVectorVectorParameterSetID;

//  art::RNGsnapshot                             dummyRNGsnap;
//  std::vector<art::RNGsnapshot>                dummyVectorRNGsnap;
//  art::Wrapper<std::vector<art::RNGsnapshot> > dummyWrapperVectorRNGsnap;

  // The remaining ones are for backward compatibility only.
  std::map<art::ModuleDescriptionID, art::ModuleDescription> dummyMapMod;
  std::pair<art::ModuleDescriptionID, art::ModuleDescription> dummyPairMod;
  std::vector<art::EventProcessHistoryID> dummyEventProcessHistory;
  art::EntryDescriptionID dummyEntryDescriptionID;
  std::vector<art::EventEntryInfo> dummyVectorEventEntryInfo;
  std::vector<art::RunSubRunEntryInfo> dummyVectorRunSubRunEntryInfo;
};
}
