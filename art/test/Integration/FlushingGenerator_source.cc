#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/Source.h"
#include "art/Framework/IO/Sources/SourceHelper.h"
#include "art/Framework/IO/Sources/SourceTraits.h"

#include <cassert>
#include <iostream>

namespace arttest {
  class FlushingGeneratorDetail;
}

class arttest::FlushingGeneratorDetail {
public:
  FlushingGeneratorDetail(FlushingGeneratorDetail const&) = delete;
  FlushingGeneratorDetail& operator=(FlushingGeneratorDetail const&) = delete;

  FlushingGeneratorDetail(fhicl::ParameterSet const& ps,
                          art::ProductRegistryHelper& help,
                          art::SourceHelper const& sHelper);

  void closeCurrentFile();

  void readFile(std::string const& name, art::FileBlock*& fb);

  bool readNext(art::RunPrincipal* const inR,
                art::SubRunPrincipal* const inSR,
                art::RunPrincipal*& outR,
                art::SubRunPrincipal*& outSR,
                art::EventPrincipal*& outE);

private:
  bool readFileCalled_;
  art::SourceHelper const& sHelper_;
  size_t ev_num_;
  art::EventID curr_evid_;
};

arttest::FlushingGeneratorDetail::FlushingGeneratorDetail(
  fhicl::ParameterSet const&,
  art::ProductRegistryHelper&,
  art::SourceHelper const& sHelper)
  : readFileCalled_(false), sHelper_(sHelper), ev_num_(0), curr_evid_(1, 0, 1)
{}

void
arttest::FlushingGeneratorDetail::closeCurrentFile()
{}

void
arttest::FlushingGeneratorDetail::readFile(std::string const& name,
                                           art::FileBlock*& fb)
{
  assert(!readFileCalled_);
  assert(name.empty());
  readFileCalled_ = true;
  fb = new art::FileBlock(art::FileFormatVersion(1, "FlushingGenerator2012"),
                          "nothing");
}

bool
arttest::FlushingGeneratorDetail::readNext(art::RunPrincipal* const inR,
                                           art::SubRunPrincipal* const inSR,
                                           art::RunPrincipal*& outR,
                                           art::SubRunPrincipal*& outSR,
                                           art::EventPrincipal*& outE)
{
  art::Timestamp runstart;
  if (++ev_num_ > 17) { // Done.
    return false;
  } else if (ev_num_ == 3) { // Flush subrun, current run.
    art::EventID const evid(art::EventID::flushEvent(inR->runID()));
    outSR = sHelper_.makeSubRunPrincipal(evid.subRunID(), runstart);
    outE = sHelper_.makeEventPrincipal(evid, runstart);
    curr_evid_ = curr_evid_.nextSubRun();
  } else if (ev_num_ == 6) { // Flush run.
    art::EventID const evid{art::EventID::flushEvent()};
    outR = sHelper_.makeRunPrincipal(evid.runID(), runstart);
    outSR = sHelper_.makeSubRunPrincipal(evid.subRunID(), runstart);
    outE = sHelper_.makeEventPrincipal(evid, runstart);
    curr_evid_ = curr_evid_.nextRun();
  } else if (ev_num_ == 11 || ev_num_ == 12 ||
             ev_num_ == 16) { // Flush event. current run, next subrun
    outSR = sHelper_.makeSubRunPrincipal(curr_evid_.nextSubRun().subRunID(),
                                         runstart);
    art::EventID const evid(art::EventID::flushEvent(outSR->subRunID()));
    outE = sHelper_.makeEventPrincipal(evid, runstart);
    curr_evid_ = curr_evid_.nextSubRun();
  } else {
    if (inR == nullptr || inR->runID() != curr_evid_.runID()) {
      outR = sHelper_.makeRunPrincipal(curr_evid_.runID(), runstart);
    }
    if (inSR == nullptr || inSR->subRunID() != curr_evid_.subRunID()) {
      outSR = sHelper_.makeSubRunPrincipal(curr_evid_.subRunID(), runstart);
    }
    outE = sHelper_.makeEventPrincipal(curr_evid_, runstart);
    curr_evid_ = curr_evid_.next();
  }
  std::cout << "ev_num_ = " << ev_num_ << std::endl;
  return true;
}

// Trait definition (must precede source typedef).
namespace art {
  template <>
  struct Source_generator<arttest::FlushingGeneratorDetail> {
    static constexpr bool value = true;
  };
} // namespace art

// Source declaration.
namespace arttest {
  typedef art::Source<FlushingGeneratorDetail> FlushingGenerator;
}

DEFINE_ART_INPUT_SOURCE(arttest::FlushingGenerator)
