#ifndef ART_FRAMEWORK_CORE_INTERMEDIATETABLEPOSTPROCESSOR_H
#define ART_FRAMEWORK_CORE_INTERMEDIATETABLEPOSTPROCESSOR_H

namespace art {
   class IntermediateTablePostProcessor;
}

namespace fhicl {
   class ParameterSet;
   class intermediate_table;
}

struct art::IntermediateTablePostProcessor {
   bool operator() (fhicl::intermediate_table &raw_config,
                    fhicl::ParameterSet & main_pset);
};
#endif
