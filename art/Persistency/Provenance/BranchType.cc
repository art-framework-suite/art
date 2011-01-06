// ======================================================================
//
// BranchType.cc
//
// ======================================================================


// framework support:
#include "art/Persistency/Provenance/BranchType.h"


namespace art {

  namespace {

    // Suffixes
#ifdef FW_BACKWARD_COMPATIBILITY
    std::string const aux                      = "Aux";
#endif
    std::string const auxiliary                = "Auxiliary";
    std::string const branchEntryInfo          = "BranchEntryInfo";
    std::string const majorIndex               = ".id_.run_";
    std::string const metaData                 = "MetaData";
    std::string const productStatus            = "ProductStatus";
    std::string const statusInformation        = "StatusInformation";

    // Prefixes
    std::string const run                      = "Run";
    std::string const subRun                     = "SubRun";
    std::string const event                    = "Event";

    // Trees, branches, indices
    std::string const runs                     = run   + 's';
    std::string const subRuns                    = subRun  + 's';
    std::string const events                   = event + 's';

    std::string const runMeta                  = run   + metaData;
    std::string const subRunMeta                 = subRun  + metaData;
    std::string const eventMeta                = event + metaData;

#ifdef FW_BACKWARD_COMPATIBILITY
    std::string const runInfo                  = run   + statusInformation;
    std::string const subRunInfo                 = subRun  + statusInformation;
    std::string const eventInfo                = event + statusInformation;
#endif

    std::string const runAuxiliary             = run   + auxiliary;
    std::string const subRunAuxiliary            = subRun  + auxiliary;
    std::string const eventAuxiliary           = event + auxiliary;

#ifdef FW_BACKWARD_COMPATIBILITY
    std::string const runProductStatus         = run   + productStatus;
    std::string const subRunProductStatus        = subRun  + productStatus;
    std::string const eventProductStatus       = event + productStatus;
#endif

    std::string const runEventEntryInfo        = run   + branchEntryInfo;
    std::string const subRunEventEntryInfo       = subRun  + branchEntryInfo;
    std::string const eventEventEntryInfo      = event + branchEntryInfo;

    std::string const runMajorIndex            = runAuxiliary   + majorIndex;
    std::string const subRunMajorIndex           = subRunAuxiliary  + majorIndex;
    std::string const eventMajorIndex          = eventAuxiliary + majorIndex;

    std::string const runMinorIndex;           // empty
    std::string const subRunMinorIndex           = subRunAuxiliary  + ".id_.subRun_";
    std::string const eventMinorIndex          = eventAuxiliary + ".id_.event_";

#ifdef FW_BACKWARD_COMPATIBILITY
    std::string const runAux                   = run   + aux;
    std::string const subRunAux                  = subRun  + aux;
    std::string const eventAux                 = event + aux;
#endif

#ifdef FW_OBSOLETE
    std::string const entryDescriptionTree     = "EntryDescription";
    std::string const entryDescriptionIDBranch = "Hash";
    std::string const entryDescriptionBranch   = "Description";
#endif

    std::string const parentageTree            = "Parentage";
    std::string const parentageIDBranch        = "Hash";
    std::string const parentageBranch          = "Description";

    std::string const metaDataTree             = "MetaData";
    std::string const productRegistry          = "ProductRegistry";
    std::string const productDependencies      = "ProductDependencies";
    std::string const parameterSetMap          = "ParameterSetMap";
    std::string const moduleDescriptionMap     = "ModuleDescriptionMap";
    std::string const processHistoryMap        = "ProcessHistoryMap";
    std::string const processConfigurationMap  = "ProcessConfigurationMap";
    std::string const branchIDLists            = "BranchIDLists";
    std::string const fileFormatVersion        = "FileFormatVersion";
    std::string const fileIdentifier           = "FileIdentifier";
    std::string const fileIndex                = "FileIndex";
    std::string const eventHistory             = "EventHistory";
    std::string const eventBranchMapper        = "EventBranchMapper";

    inline
    std::string const & select( BranchType          bt
                              , std::string const & event_str
                              , std::string const & run_str
                              , std::string const & subRun_str
                              )
    {
      switch( bt ) {
        case InEvent:  return event_str;
        case InRun  :  return run_str;
        case InSubRun :  return subRun_str;
        default     :  return subRun_str;  // TODO: report "none of the above"?
      }
    }

  }  // namespace


  std::string const & BranchTypeToString( BranchType bt ) {
    select( bt, event, run, subRun );
  }

  std::string const & BranchTypeToProductTreeName( BranchType bt ) {
    select( bt, events, runs, subRuns );
  }

  std::string const & BranchTypeToMetaDataTreeName( BranchType bt ) {
    select( bt, eventMeta, runMeta, subRunMeta );
  }

#ifdef FW_BACKWARD_COMPATIBILITY
  std::string const & BranchTypeToInfoTreeName( BranchType bt ) {
    select( bt, eventInfo, runInfo, subRunInfo );
  }
#endif

  std::string const & BranchTypeToAuxiliaryBranchName( BranchType bt ) {
    select( bt, eventAuxiliary, runAuxiliary, subRunAuxiliary );
  }

#ifdef FW_BACKWARD_COMPATIBILITY
  std::string const & BranchTypeToAuxBranchName( BranchType bt ) {
    select( bt, eventAux, runAux, subRunAux );
  }
#endif

#ifdef FW_BACKWARD_COMPATIBILITY
  std::string const & BranchTypeToProductStatusBranchName( BranchType bt ) {
    select( bt, eventProductStatus, runProductStatus, subRunProductStatus );
  }
#endif

  std::string const & BranchTypeToBranchEntryInfoBranchName( BranchType bt ) {
    select( bt, eventEventEntryInfo, runEventEntryInfo, subRunEventEntryInfo );
  }

  std::string const & BranchTypeToMajorIndexName( BranchType bt ) {
    select( bt, eventMajorIndex, runMajorIndex, subRunMajorIndex );
  }

  std::string const & BranchTypeToMinorIndexName( BranchType bt ) {
    select( bt, eventMinorIndex, runMinorIndex, subRunMinorIndex );
  }

  namespace rootNames {

#ifdef FW_OBSOLETE
    // EntryDescription tree (1 entry per recorded distinct value of EntryDescription)
    std::string const & entryDescriptionTreeName( ) {
      return entryDescriptionTree;
    }

    std::string const & entryDescriptionIDBranchName( ) {
      return entryDescriptionIDBranch;
    }

    std::string const & entryDescriptionBranchName( ) {
      return entryDescriptionBranch;
    }
#endif

    // EntryDescription tree (1 entry per recorded distinct value of EntryDescription)
    std::string const & parentageTreeName( ) {
      return parentageTree;
    }

    std::string const & parentageIDBranchName( ) {
      return parentageIDBranch;
    }

    std::string const & parentageBranchName( ) {
      return parentageBranch;
    }

    // MetaData Tree (1 entry per file)
    std::string const & metaDataTreeName( ) {
      return metaDataTree;
    }

    // Branch on MetaData Tree
    std::string const & productDescriptionBranchName( ) {
      return productRegistry;
    }

    // Branch on MetaData Tree
    std::string const & productDependenciesBranchName( ) {
      return productDependencies;
    }

    // Branch on MetaData Tree
    std::string const & parameterSetMapBranchName( ) {
      return parameterSetMap;
    }

#ifdef FW_OBSOLETE
    // Branch on MetaData Tree
    std::string const & moduleDescriptionMapBranchName( ) {
      return moduleDescriptionMap;
    }
#endif

    // Branch on MetaData Tree
    std::string const & processHistoryMapBranchName( ) {
      return processHistoryMap;
    }

    // Branch on MetaData Tree
    std::string const & processConfigurationBranchName( ) {
      return processConfigurationMap;
    }

    // Branch on MetaData Tree
    std::string const & branchIDListBranchName( ) {
      return branchIDLists;
    }

    // Branch on MetaData Tree
    std::string const & fileFormatVersionBranchName( ) {
      return fileFormatVersion;
    }

    // Branch on MetaData Tree
    std::string const & fileIdentifierBranchName( ) {
      return fileIdentifier;
    }

    // Branch on MetaData Tree
    std::string const & fileIndexBranchName( ) {
      return fileIndex;
    }

    // Branch on Event History Tree
    std::string const & eventHistoryBranchName( ) {
      return eventHistory;
    }

    std::string const & eventTreeName( ) {
      return events;
    }

    std::string const & eventMetaDataTreeName( ) {
      return eventMeta;
    }

    std::string const & eventHistoryTreeName( ) {
      return eventHistory;
    }
  }

}  // art

// ======================================================================
