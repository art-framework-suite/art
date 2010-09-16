#ifndef ParameterSet_ParameterSetEntry_h
#define ParameterSet_ParameterSetEntry_h

/** How ParameterSets are nested inside ParameterSets
    The main feature is that they're made persistent
    using a ParameterSetID, and only reconstituted as needed,
    when the value_ptr = 0;
  */


#include "art/Utilities/value_ptr.h"

#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"


namespace edm {

  class ParameterSetEntry
  {
  public:
    // default ctor for serialization
    ParameterSetEntry();
    ParameterSetEntry(const fhicl::ParameterSet & pset, bool isTracked);
    ParameterSetEntry(const std::string & rep);

    ~ParameterSetEntry();

    std::string toString() const;
    int sizeOfString() const;

    bool isTracked() const {return tracked;}

    fhicl::ParameterSetID id() const {return theID;}

    /// returns the PSet, reconstituting it from the
    /// Registry, if necessary
    const fhicl::ParameterSet & pset() const;
    fhicl::ParameterSet & pset();

    /// we expect this to only be called by ParameterSet, on tracked psets
    void updateID() const;

    friend std::ostream & operator<<(std::ostream & os, ParameterSetEntry const& psetEntry);

  private:

    bool tracked;
    // can be internally reconstituted from the ID, in an
    // ostensibly const function
    mutable value_ptr<fhicl::ParameterSet> thePSet;

    // mutable so save() can serialize it as late as possible
    mutable fhicl::ParameterSetID theID;


  };

}  // namespace edm

#endif  // ParameterSet_ParameterSetEntry_h
