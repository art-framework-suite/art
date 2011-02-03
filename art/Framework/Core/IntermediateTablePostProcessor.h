#ifndef art_Framework_Core_IntermediateTablePostProcessor_h
#define art_Framework_Core_IntermediateTablePostProcessor_h

namespace art {
   class IntermediateTablePostProcessor;
}

namespace fhicl {
   class intermediate_table;
}

struct art::IntermediateTablePostProcessor {
   void apply(fhicl::intermediate_table &raw_config) const;
};
#endif /* art_Framework_Core_IntermediateTablePostProcessor_h */

// Local Variables:
// mode: c++
// End:
