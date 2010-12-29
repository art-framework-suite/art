#ifndef DataFormats_Provenance_HashedTypes_h
#define DataFormats_Provenance_HashedTypes_h

// ======================================================================
//
// enum HashedTypes, used in defining several "id" classes.
//
// ======================================================================

namespace art
{
  enum HashedTypes {
      ModuleDescriptionType, // Obsolete
      ParameterSetType,
      ProcessHistoryType,
      ProcessConfigurationType,
      EntryDescriptionType, // Obsolete
      ParentageType
  };
}

// ======================================================================

#endif
