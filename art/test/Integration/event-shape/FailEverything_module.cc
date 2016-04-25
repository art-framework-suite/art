#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"

#include "fhiclcpp/types/Atom.h"

#include <cassert>

namespace {
  using namespace fhicl;
  struct Config {};
}

namespace arttest {

  class FailEverything : public art::EDFilter {
  public:

    using Parameters = EDFilter::Table<Config>;
    explicit FailEverything(EDFilter::Table<Config> const&){}

    bool filter(art::Event&) override { return EDFilter::Fail; }
  };

}

DEFINE_ART_MODULE(arttest::FailEverything)
