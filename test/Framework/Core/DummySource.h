#ifndef Modules_DummySource_h
#define Modules_DummySource_h

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
#endif
