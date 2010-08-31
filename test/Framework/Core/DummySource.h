#ifndef Modules_DummySource_h
#define Modules_DummySource_h

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/Core/GeneratedInputSource.h"

namespace edm {
  class DummySource : public GeneratedInputSource {
  public:
    explicit DummySource(ParameterSet const&, InputSourceDescription const&);
    ~DummySource();
  private:
    virtual bool produce(Event &);
  };
}
#endif
