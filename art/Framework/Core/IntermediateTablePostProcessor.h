#ifndef art_Framework_Core_IntermediateTablePostProcessor_h
#define art_Framework_Core_IntermediateTablePostProcessor_h

namespace art {
  class IntermediateTablePostProcessor;
}

namespace fhicl {
  class intermediate_table;
}

class art::IntermediateTablePostProcessor {
public:
  IntermediateTablePostProcessor();
  void apply(fhicl::intermediate_table & raw_config) const;

  void wantRethrowDefault() { rethrowDefault_ = true; }
  void wantRethrowAll() { rethrowAll_ = true; }
private:
  void applyRethrowOptions(fhicl::intermediate_table & raw_config) const;

  bool rethrowDefault_;
  bool rethrowAll_;
};
#endif /* art_Framework_Core_IntermediateTablePostProcessor_h */

// Local Variables:
// mode: c++
// End:
