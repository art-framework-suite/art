#ifndef test_Framework_Core_DummySource_h
#define test_Framework_Core_DummySource_h

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/Core/GeneratedInputSource.h"

namespace art {
  class DummySource : public GeneratedInputSource {
  public:
      explicit DummySource(fhicl::ParameterSet const&, InputSourceDescription const&);
    ~DummySource();
  private:
    virtual bool produce(Event &);
  };
}
#endif /* test_Framework_Core_DummySource_h */

// Local Variables:
// mode: c++
// End:
