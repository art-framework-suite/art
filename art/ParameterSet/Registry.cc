// ----------------------------------------------------------------------
//
// Registry
//
// ----------------------------------------------------------------------


#include "art/ParameterSet/Registry.h"

#include "art/Utilities/EDMException.h"

using fhicl::ParameterSet;


namespace art {
  namespace pset {

    bool
    insertParameterSetIntoRegistry(Registry* reg, ParameterSet const& p)
    {
      ParameterSet tracked_part = p.trackedPart();
      return reg->insertMapped(tracked_part);
    }

    void
    loadAllNestedParameterSets(Registry* reg, ParameterSet const& main)
    {
      std::vector<ParameterSet> all_main_psets;
      explode(main, all_main_psets);
      std::vector<ParameterSet>::const_iterator i = all_main_psets.begin();
      std::vector<ParameterSet>::const_iterator e = all_main_psets.end();
      for (; i != e; ++i) reg->insertMapped(*i);
      reg->extra().setID(main.id());
    }

    fhicl::ParameterSetID
    getProcessParameterSetID(Registry const* reg)
    {
      return reg->extra().id();
    }

    void fill(Registry* reg, regmap_type& fillme)
    {
      typedef Registry::const_iterator iter;
      fillme.clear();
      for (iter i=reg->begin(), e=reg->end(); i!=e; ++i)
        fillme[i->first].pset_ = i->second.toStringOfTracked();
    }

  } // namespace pset

  fhicl::ParameterSet getProcessParameterSet()
  {
    art::pset::Registry* reg = art::pset::Registry::instance();
    fhicl::ParameterSetID id = art::pset::getProcessParameterSetID(reg);

    fhicl::ParameterSet result;
    if (!reg->getMapped(id, result))
      throw art::Exception(errors::EventCorruption, "Unknown ParameterSetID")
        << "Unable to find the ParameterSet for id: "
        << id
        << ";\nthis was supposed to be the process ParameterSet\n";

    return result;
  }

}  // namespace art
