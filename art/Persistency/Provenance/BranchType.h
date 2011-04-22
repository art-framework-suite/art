#ifndef art_Persistency_Provenance_BranchType_h
#define art_Persistency_Provenance_BranchType_h

// ======================================================================
//
// BranchType: enumerate/encode/label the three Branch types
//
// ======================================================================

#include <string>

// identify outdated components for future removal:
#define FW_BACKWARD_COMPATIBILITY

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

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_BranchType_h */

// Local Variables:
// mode: c++
// End:
