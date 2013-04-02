#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/PrincipalMaker.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/Source.h"
#include "art/Framework/IO/Sources/SourceTraits.h"

#include <cassert>

namespace arttest {
  class FlushingGeneratorDetail;
}

class arttest::FlushingGeneratorDetail {
public:
  FlushingGeneratorDetail(FlushingGeneratorDetail const &) = delete;
  FlushingGeneratorDetail & operator =(FlushingGeneratorDetail const &) = delete;

  FlushingGeneratorDetail(fhicl::ParameterSet const & ps,
                      art::ProductRegistryHelper & help,
                      art::PrincipalMaker const & pm);

  void closeCurrentFile();

  void readFile(std::string const & name, art::FileBlock *& fb);

  bool readNext(art::RunPrincipal * const & inR,
                art::SubRunPrincipal * const & inSR,
                art::RunPrincipal *& outR,
                art::SubRunPrincipal *& outSR,
                art::EventPrincipal *& outE);

private:
  bool readFileCalled_;
  art::PrincipalMaker const & pm_;
  size_t ev_num_;
};

arttest::FlushingGeneratorDetail::
FlushingGeneratorDetail(fhicl::ParameterSet const &,
                    art::ProductRegistryHelper &,
                    art::PrincipalMaker const & pm)
  :
  readFileCalled_(false),
  pm_(pm),
  ev_num_(0)
{
}

void
arttest::FlushingGeneratorDetail::
closeCurrentFile()
{
}

void
arttest::FlushingGeneratorDetail::
readFile(std::string const & name, art::FileBlock *& fb)
{
  assert(!readFileCalled_);
  assert(name.empty());
  readFileCalled_ = true;
  fb = new art::FileBlock(art::FileFormatVersion(1, "FlushingGenerator2012"), "nothing");
}

bool
arttest::FlushingGeneratorDetail::
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
    outR = pm_.makeRunPrincipal(1, runstart);
  }
  if (inSR == nullptr) {
    art::SubRunID srid(outR?outR->run():inR->run(), 0);
    outSR = pm_.makeSubRunPrincipal(srid.run(), srid.subRun(), runstart);
  }
  outE = pm_.makeEventPrincipal(outR?outR->run():inR->run(),
                                outSR?outSR->subRun():inSR->subRun(),
                                ev_num_,
                                runstart);
  return true;
}

// Trait definition (must precede source typedef).
namespace art {
  template <>
    struct Source_generator<arttest::FlushingGeneratorDetail> {
    static constexpr bool value = true;
  };
}

// Source declaration.
namespace arttest {
  typedef art::Source<FlushingGeneratorDetail> FlushingGenerator;
}

DEFINE_ART_INPUT_SOURCE(arttest::FlushingGenerator)
