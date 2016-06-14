#ifndef art_Persistency_Provenance_BranchType_h
#define art_Persistency_Provenance_BranchType_h

// ======================================================================
//
// BranchType: enumerate/encode/label the three Branch types
//
// ======================================================================

#include <string>

// ----------------------------------------------------------------------

namespace art {

  // Note: These enum values are used as subscripts for a fixed size array,
  // so they must not change.
  enum BranchType { InEvent,
                    InSubRun,
                    InRun,
                    InResults,
                    NumBranchTypes
                  };

  std::string const & BranchTypeToString( BranchType );

  std::string const & BranchTypeToProductTreeName( BranchType );

  std::string const & BranchTypeToMetaDataTreeName( BranchType );

  std::string const & BranchTypeToAuxiliaryBranchName( BranchType );

  std::string const & productProvenanceBranchName( BranchType );

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
