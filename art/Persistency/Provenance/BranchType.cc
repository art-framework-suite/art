// ======================================================================
//
// BranchType.cc
//
// ======================================================================


// framework support:
#include "art/Persistency/Provenance/BranchType.h"


namespace edm {

  namespace {

    // Suffixes
#ifdef FW_BACKWARD_COMPATIBILITY
    std::string const aux                      = "Aux";
#endif  // FW_BACKWARD_COMPATIBILITY
    std::string const auxiliary                = "Auxiliary";
    std::string const branchEntryInfo          = "BranchEntryInfo";
    std::string const majorIndex               = ".id_.run_";
    std::string const metaData                 = "MetaData";
    std::string const productStatus            = "ProductStatus";
    std::string const statusInformation        = "StatusInformation";

    // Prefixes
    std::string const run                      = "Run";
    std::string const lumi                     = "SubRun";
    std::string const event                    = "Event";

    // Trees, branches, indices
    std::string const runs                     = run   + 's';
    std::string const lumis                    = lumi  + 's';
    std::string const events                   = event + 's';

    std::string const runMeta                  = run   + metaData;
    std::string const lumiMeta                 = lumi  + metaData;
    std::string const eventMeta                = event + metaData;

#ifdef FW_BACKWARD_COMPATIBILITY
    std::string const runInfo                  = run   + statusInformation;
    std::string const lumiInfo                 = lumi  + statusInformation;
    std::string const eventInfo                = event + statusInformation;
#endif  // FW_BACKWARD_COMPATIBILITY

    std::string const runAuxiliary             = run   + auxiliary;
    std::string const lumiAuxiliary            = lumi  + auxiliary;
    std::string const eventAuxiliary           = event + auxiliary;

#ifdef FW_BACKWARD_COMPATIBILITY
    std::string const runProductStatus         = run   + productStatus;
    std::string const lumiProductStatus        = lumi  + productStatus;
    std::string const eventProductStatus       = event + productStatus;
#endif  // FW_BACKWARD_COMPATIBILITY

    std::string const runEventEntryInfo        = run   + branchEntryInfo;
    std::string const lumiEventEntryInfo       = lumi  + branchEntryInfo;
    std::string const eventEventEntryInfo      = event + branchEntryInfo;

    std::string const runMajorIndex            = runAuxiliary   + majorIndex;
    std::string const lumiMajorIndex           = lumiAuxiliary  + majorIndex;
    std::string const eventMajorIndex          = eventAuxiliary + majorIndex;

    std::string const runMinorIndex;           // empty
    std::string const lumiMinorIndex           = lumiAuxiliary  + ".id_.luminosityBlock_";
    std::string const eventMinorIndex          = eventAuxiliary + ".id_.event_";

#ifdef FW_BACKWARD_COMPATIBILITY
    std::string const runAux                   = run   + aux;
    std::string const lumiAux                  = lumi  + aux;
    std::string const eventAux                 = event + aux;
#endif  // FW_BACKWARD_COMPATIBILITY

#ifdef FW_OBSOLETE
    std::string const entryDescriptionTree     = "EntryDescription";
    std::string const entryDescriptionIDBranch = "Hash";
    std::string const entryDescriptionBranch   = "Description";
#endif  // FW_OBSOLETE

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
                              , std::string const & lumi_str
                              )
    {
      switch( bt ) {
        case InEvent:  return event_str;
        case InRun  :  return run_str;
        case InSubRun :  return lumi_str;
        default     :  return lumi_str;  // TODO: report "none of the above"?
      }
    }

  }  // namespace


  std::string const & BranchTypeToString( BranchType bt ) {
    select( bt, event, run, lumi );
  }

  std::string const & BranchTypeToProductTreeName( BranchType bt ) {
    select( bt, events, runs, lumis );
  }

  std::string const & BranchTypeToMetaDataTreeName( BranchType bt ) {
    select( bt, eventMeta, runMeta, lumiMeta );
  }

#ifdef FW_BACKWARD_COMPATIBILITY
  std::string const & BranchTypeToInfoTreeName( BranchType bt ) {
    select( bt, eventInfo, runInfo, lumiInfo );
  }
#endif  // FW_BACKWARD_COMPATIBILITY

  std::string const & BranchTypeToAuxiliaryBranchName( BranchType bt ) {
    select( bt, eventAuxiliary, runAuxiliary, lumiAuxiliary );
  }

#ifdef FW_BACKWARD_COMPATIBILITY
  std::string const & BranchTypeToAuxBranchName( BranchType bt ) {
    select( bt, eventAux, runAux, lumiAux );
  }
#endif  // FW_BACKWARD_COMPATIBILITY

#ifdef FW_BACKWARD_COMPATIBILITY
  std::string const & BranchTypeToProductStatusBranchName( BranchType bt ) {
    select( bt, eventProductStatus, runProductStatus, lumiProductStatus );
  }
#endif  // FW_BACKWARD_COMPATIBILITY

  std::string const & BranchTypeToBranchEntryInfoBranchName( BranchType bt ) {
    select( bt, eventEventEntryInfo, runEventEntryInfo, lumiEventEntryInfo );
  }

  std::string const & BranchTypeToMajorIndexName( BranchType bt ) {
    select( bt, eventMajorIndex, runMajorIndex, lumiMajorIndex );
  }

  std::string const & BranchTypeToMinorIndexName( BranchType bt ) {
    select( bt, eventMinorIndex, runMinorIndex, lumiMinorIndex );
  }

  namespace poolNames {

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
#endif  // FW_OBSOLETE

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
#endif  // FW_OBSOLETE

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
}
