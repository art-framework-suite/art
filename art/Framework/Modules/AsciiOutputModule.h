/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include <iostream>
#include <string>
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/OutputModule.h"


namespace edm {
  class ParameterSet;
  class AsciiOutputModule : public OutputModule {
  public:
    // We do not take ownership of passed stream.
    explicit AsciiOutputModule(ParameterSet const& pset);
    virtual ~AsciiOutputModule();

  private:
    virtual void write(EventPrincipal const& e);
    virtual void writeSubRun(SubRunPrincipal const&){}
    virtual void writeRun(RunPrincipal const&){}
    int prescale_;
    int verbosity_;
    int counter_;
  };
}
