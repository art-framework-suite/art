#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"

#include "TError.h"
#include "TFile.h"

#include <cassert>

namespace {

  class XRootDRetry : public art::EDAnalyzer {
  public:
    struct Config {};
    using Parameters = Table<Config>;
    explicit XRootDRetry(Parameters const& p);

  private:
    void
    analyze(art::Event const&) override
    {}
  };

  XRootDRetry::XRootDRetry(Parameters const& p) : EDAnalyzer{p}
  {
    // We test here if art's custom error handler turns a "Temporary
    // failure in name resolution" error into a fatal error.  It
    // should not.
    Error("TUnixSystem::GetHostByName",
          "getaddrinfo failed for '%s': %s",
          "http://fndca1.fnal.gov/",
          "Temporary failure in name resolution");

    // Test fatal error
    try {
      TFile::Open("xroot://a.b.c//gibberish.root");
      assert(false);
    }
    catch (...) {
    }
  }
}

DEFINE_ART_MODULE(XRootDRetry)
