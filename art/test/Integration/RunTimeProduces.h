#ifndef art_test_Integration_RunTimeProduces_h
#define art_test_Integration_RunTimeProduces_h

#include "canvas/Persistency/Provenance/BranchType.h"
#include <string>

namespace art {
  namespace test {

    template <typename T>
    void
    run_time_produces(art::EDProducer* p,
                      art::BranchType const bt,
                      std::string const& instance = {})
    {
      assert(p != nullptr);
      switch (bt) {
        case art::InEvent:
          p->produces<T>(instance);
          break;
        case art::InSubRun:
          p->produces<T, art::InSubRun>(instance);
          break;
        case art::InRun:
          p->produces<T, art::InRun>(instance);
          break;
        default:
          throw art::Exception(art::errors::LogicError)
            << "Unknown branch type " << bt << ".\n";
      }
    }
  }
}

#endif /* art_test_Integration_RunTimeProduces_h */

// Local Variables:
// mode: c++
// End:
