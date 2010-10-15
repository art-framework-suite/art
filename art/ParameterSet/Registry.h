#ifndef FWCore_ParameterSet_Registry_h
#define FWCore_ParameterSet_Registry_h

// ----------------------------------------------------------------------
//
// Declaration for pset::Registry. This is an implementation detail of
// the ParameterSet library.
//
// A Registry is used to keep track of the persistent form of all
// ParameterSets used a given program, so that they may be retrieved by
// ParameterSetID, and so they may be written to persistent storage.
//
// ----------------------------------------------------------------------


#include "art/Persistency/Provenance/ParameterSetBlob.h"
#include "art/Utilities/ThreadSafeRegistry.h"

#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"

#include <map>


namespace art {

  namespace pset {

    class ProcessParameterSetIDCache
    {
    public:
      ProcessParameterSetIDCache() : id_() { }
      fhicl::ParameterSetID id() const { return id_; }
      void setID(fhicl::ParameterSetID const& id) { id_ = id; }
    private:
      fhicl::ParameterSetID id_;
    };

    typedef art::detail::ThreadSafeRegistry<fhicl::ParameterSetID,
                                            fhicl::ParameterSet,
                                            ProcessParameterSetIDCache>
                                            Registry;

    /// Associated free functions.

    /// Insert the *tracked parts* of the given ParameterSet into the
    /// Registry. If there was already a ParameterSet with the same
    /// ID, we don't change itw. This should be OK, since it should
    /// have the same contents if the ID is the same.
    /// Return 'true' if we really added the new ParameterSet, and
    /// 'false' if the ParameterSet was already present.

    bool insertParameterSetIntoRegistry(Registry* reg,
                                        fhicl::ParameterSet const& p);

    void loadAllNestedParameterSets(Registry* reg,
                                    fhicl::ParameterSet const& main);


    /// Return the ParameterSetID of the top-level ParameterSet stored
    /// in the given Registry. Note the the returned ParameterSetID may
    /// be invalid; this will happen if the Registry has not yet been
    /// filled.
    fhicl::ParameterSetID getProcessParameterSetID(Registry const* reg);

    /// Fill the given map with the persistent form of each
    /// ParameterSet in the given registry.
    typedef std::map<fhicl::ParameterSetID, art::ParameterSetBlob> regmap_type;
    void fill(Registry* reg, regmap_type& fillme);

  }  // namespace pset

  fhicl::ParameterSet getProcessParameterSet();

}  // namespace art


#endif  // FWCore_ParameterSet_Registry_h
