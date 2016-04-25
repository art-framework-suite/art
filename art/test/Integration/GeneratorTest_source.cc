#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/IO/Sources/SourceHelper.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/Source.h"
#include "art/Framework/IO/Sources/SourceTraits.h"

#include <cassert>

namespace arttest {
  class GeneratorTestDetail;
}

class arttest::GeneratorTestDetail {
public:
  GeneratorTestDetail(GeneratorTestDetail const &) = delete;
  GeneratorTestDetail & operator =(GeneratorTestDetail const &) = delete;

  GeneratorTestDetail(fhicl::ParameterSet const & ps,
                      art::ProductRegistryHelper & help,
                      art::SourceHelper const & sHelper);

  void closeCurrentFile();

  void readFile(std::string const & name, art::FileBlock *& fb);

  bool readNext(art::RunPrincipal * const & inR,
                art::SubRunPrincipal * const & inSR,
                art::RunPrincipal *& outR,
                art::SubRunPrincipal *& outSR,
                art::EventPrincipal *& outE);

private:
  bool readFileCalled_;
  art::SourceHelper const & sHelper_;
  size_t ev_num_;
};

arttest::GeneratorTestDetail::
GeneratorTestDetail(fhicl::ParameterSet const &,
                    art::ProductRegistryHelper &,
                    art::SourceHelper const & sHelper)
  :
  readFileCalled_(false),
  sHelper_(sHelper),
  ev_num_(0)
{
}

void
arttest::GeneratorTestDetail::
closeCurrentFile()
{
}

void
arttest::GeneratorTestDetail::
readFile(std::string const & name, art::FileBlock *& fb)
{
  assert(!readFileCalled_);
  assert(name.empty());
  readFileCalled_ = true;
  fb = new art::FileBlock(art::FileFormatVersion(1, "GeneratorTest2012"), "nothing");
}

bool
arttest::GeneratorTestDetail::
readNext(art::RunPrincipal * const & inR,
         art::SubRunPrincipal * const & inSR,
         art::RunPrincipal *& outR,
         art::SubRunPrincipal *& outSR,
         art::EventPrincipal *& outE)
{
  art::Timestamp runstart;
  if (++ev_num_ > 5) {
    return false;
  }
  if (inR == nullptr) {
    outR = sHelper_.makeRunPrincipal(1, runstart);
  }
  if (inSR == nullptr) {
    art::SubRunID srid(outR?outR->run():inR->run(), 0);
    outSR = sHelper_.makeSubRunPrincipal(srid.run(), srid.subRun(), runstart);
  }
  outE = sHelper_.makeEventPrincipal(outR?outR->run():inR->run(),
                                outSR?outSR->subRun():inSR->subRun(),
                                ev_num_,
                                runstart);
  return true;
}

// Trait definition (must precede source typedef).
namespace art {
  template <>
    struct Source_generator<arttest::GeneratorTestDetail> {
    static constexpr bool value = true;
  };
}

// Source declaration.
namespace arttest {
  typedef art::Source<GeneratorTestDetail> GeneratorTest;
}

DEFINE_ART_INPUT_SOURCE(arttest::GeneratorTest)
