#include "art/Framework/IO/Root/rootNames.h"

#include "art/Persistency/Provenance/BranchType.h"

namespace {
  std::string const parentageTree            = "Parentage";
  std::string const parentageIDBranch        = "Hash";
  std::string const parentageBranch          = "Description";

  std::string const metaDataTree             = "MetaData";
  std::string const eventHistory             = "EventHistory";
  std::string const eventBranchMapper        = "EventBranchMapper";  

  std::string const events                   = "Events";  
  std::string const eventMeta                = "EventMetaData";  
}

// Parentage tree.
std::string const & art::rootNames::parentageTreeName( ) {
  return parentageTree;
}

std::string const & art::rootNames::parentageIDBranchName( ) {
  return parentageIDBranch;
}

std::string const & art::rootNames::parentageBranchName( ) {
  return parentageBranch;
}

// MetaData Tree (1 entry per file)
std::string const & art::rootNames::metaDataTreeName( ) {
  return metaDataTree;
}

// EventHistory Tree
std::string const & art::rootNames::eventHistoryTreeName( ) {
  return eventHistory;
}

// Branch on Event History Tree
std::string const & art::rootNames::eventHistoryBranchName( ) {
  return eventHistory;
}

std::string const & art::rootNames::eventTreeName( ) {
  return events;
}

std::string const & art::rootNames::eventMetaDataTreeName( ) {
  return eventMeta;
}

