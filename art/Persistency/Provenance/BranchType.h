#ifndef DataFormats_Provenance_BranchType_h
#define DataFormats_Provenance_BranchType_h

// ======================================================================
//
// BranchType: enumerate/encode/label the three Branch types
//
// ======================================================================

#include <string>

// identify outdated components for future removal:
#define FW_BACKWARD_COMPATIBILITY
#define FW_OBSOLETE

// ----------------------------------------------------------------------

namespace art {

  // Note: These enum values are used as subscripts for a fixed size array,
  // so they must not change.
  enum BranchType { InEvent = 0
                  , InSubRun  = 1
                  , InRun   = 2
                  , NumBranchTypes
                  };

  std::string const & BranchTypeToString( BranchType );

  std::string const & BranchTypeToProductTreeName( BranchType );

  std::string const & BranchTypeToMetaDataTreeName( BranchType );

#ifdef FW_BACKWARD_COMPATIBILITY
  std::string const & BranchTypeToInfoTreeName( BranchType );
#endif

  std::string const & BranchTypeToAuxiliaryBranchName( BranchType );

#ifdef FW_BACKWARD_COMPATIBILITY
  std::string const & BranchTypeToAuxBranchName( BranchType );
#endif

#ifdef FW_BACKWARD_COMPATIBILITY
  std::string const & BranchTypeToProductStatusBranchName( BranchType );
#endif

  std::string const & BranchTypeToBranchEntryInfoBranchName( BranchType );

  std::string const & BranchTypeToMajorIndexName( BranchType );

  std::string const & BranchTypeToMinorIndexName( BranchType );

  inline
  std::ostream & operator << ( std::ostream & os,  BranchType branchType ) {
    return os << BranchTypeToString(branchType);
  }

  namespace rootNames {
#ifdef FW_OBSOLETE
    //------------------------------------------------------------------
    // EntryDescription Tree
    std::string const & entryDescriptionTreeName( );

    // Branches on EntryDescription Tree
    std::string const & entryDescriptionIDBranchName( );
    std::string const & entryDescriptionBranchName( );
#endif

    //------------------------------------------------------------------
    // Parentage Tree
    std::string const & parentageTreeName( );

    // Branches on parentage tree
    std::string const & parentageIDBranchName( );
    std::string const & parentageBranchName( );

    //------------------------------------------------------------------
    //------------------------------------------------------------------
    // MetaData Tree (1 entry per file)
    std::string const & metaDataTreeName( );

    // Branches on MetaData Tree
    std::string const & productDescriptionBranchName( );
    std::string const & productDependenciesBranchName( );
    std::string const & parameterSetMapBranchName( );
#ifdef FW_OBSOLETE
    std::string const & moduleDescriptionMapBranchName( );
#endif
    std::string const & processHistoryMapBranchName( );
    std::string const & processConfigurationBranchName( );
    std::string const & branchIDListBranchName( );
    std::string const & fileFormatVersionBranchName( );
    std::string const & fileIdentifierBranchName( );
    std::string const & fileIndexBranchName( );

    // Event History Tree
    std::string const & eventHistoryTreeName( );

    // Branches on EventHistory Tree
    std::string const & eventHistoryBranchName( );

    //------------------------------------------------------------------
    // Other tree names
    std::string const & eventTreeName( );
    std::string const & eventMetaDataTreeName( );
  }  // rootNames
}  // art

// ======================================================================

#endif
