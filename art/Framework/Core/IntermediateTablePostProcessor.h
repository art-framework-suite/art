#ifndef ART_FRAMEWORK_CORE_INTERMEDIATETABLEPOSTPROCESSOR_H
#define ART_FRAMEWORK_CORE_INTERMEDIATETABLEPOSTPROCESSOR_H

namespace art {
   class IntermediateTablePostProcessor;
}

namespace fhicl {
   class intermediate_table;
}

struct art::IntermediateTablePostProcessor {
   void apply(fhicl::intermediate_table &raw_config) const;
};
#endif
