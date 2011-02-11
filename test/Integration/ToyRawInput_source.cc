
// ------------------------------------------------------------
// ToyRawInput is a RawInputSource that pretends to reconstitute
// several products.
// ------------------------------------------------------------

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/IO/Sources/RawInputSource.h"

using fhicl::ParameterSet;
using namespace art;

namespace arttest
{
  class ToyRawInput : public RawInputSource
  {
  public:

    ToyRawInput(ParameterSet const& pset,
		InputSourceDescription const& desc);

    virtual ~ToyRawInput();

  private:
    EventID nextEvent_;
  };
}

typedef std::vector<std::string> strings;
typedef std::vector<int>         ints;
typedef std::vector<double>      doubles;

arttest::ToyRawInput::ToyRawInput(ParameterSet const& pset,
				  InputSourceDescription const& desc) :
  RawInputSource(pset, desc),
  nextEvent_(EventID(1,0,1))
{
  reconstitutes<strings, InEvent>("stringmaker");
  reconstitutes<ints, InSubRun>("SUBRUNintmaker");
  reconstitutes<ints, InRun>("RUNdoublemaker");
}

arttest::ToyRawInput::~ToyRawInput()
{ }


DEFINE_ART_INPUT_SOURCE(arttest::ToyRawInput);
