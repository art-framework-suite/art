// ------------------------------------------------------------
//
// The purpose of 'DoNothingInput' is to verify that when the
// destructor is called, the art::ServiceHandle can be constructed
// successfully and not throw.  This tests the expected behavior of
// the art::EventProcessor constructor.
//
// ------------------------------------------------------------

#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/IO/Sources/Source.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"

#include "art/test/Integration/Wanted.h"

#include <memory>

namespace art {
  class FileBlock;
  class ProductRegistryHelper;
  class SourceHelper;
}

namespace fhicl {
  class ParameterSet;
}

namespace arttest {

  class DoNothingInput {
  public:

    DoNothingInput(fhicl::ParameterSet const&,
                art::ProductRegistryHelper&,
                art::SourceHelper const&)
    {}

    ~DoNothingInput()
    {
      art::ServiceHandle<Wanted> shouldNotThrow [[gnu::unused]];
    }

    void readFile(std::string const &, art::FileBlock*&)
    {}

    void closeCurrentFile()
    {}

    bool readNext(art::RunPrincipal const* const,
                  art::SubRunPrincipal const* const,
                  art::RunPrincipal*&,
                  art::SubRunPrincipal*&,
                  art::EventPrincipal*&)
    {
      return false;
    }

  };

}

DEFINE_ART_INPUT_SOURCE(art::Source<arttest::DoNothingInput>)
