#ifndef Modules_EmptySource_h
#define Modules_EmptySource_h

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/Core/GeneratedInputSource.h"

namespace edm {
  class EmptySource : public GeneratedInputSource {
  public:
    explicit EmptySource(ParameterSet const&, InputSourceDescription const&);
    ~EmptySource();
  private:
    virtual bool produce(Event &);
  };
}
#endif
