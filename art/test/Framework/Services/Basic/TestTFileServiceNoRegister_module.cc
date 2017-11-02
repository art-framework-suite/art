#include "TH1F.h"

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"

#include <string>

using namespace art;
using namespace std;

class TestTFileServiceNoRegister : public art::EDAnalyzer {
public:
  struct Config {
    fhicl::Atom<bool> expectThrow{
      fhicl::Name{"expectThrow"},
      fhicl::Comment{
        "In the case that services.TFileService.fileProperties is specified\n"
        "we expect that the c'tor of this module should throw since\n"
        "no file-switch callback has been registered."}};
  };
  using Parameters = Table<Config>;
  explicit TestTFileServiceNoRegister(Parameters const&);

private:
  void
  analyze(Event const&) override
  {}
  bool const expectThrow_;
}; // TestTFileServiceNoRegister

TestTFileServiceNoRegister::TestTFileServiceNoRegister(Parameters const& p)
  : EDAnalyzer{p}, expectThrow_{p().expectThrow()}
{
  ServiceHandle<TFileService> fs;
  // The following will throw if fileProperties is specified in the
  // configuration.
  try {
    fs->make<TH1F>("test", "Sir John", 100, 0., 100.);
    assert(!expectThrow_);
  }
  catch (art::Exception const& e) {
    assert(expectThrow_);
    assert(e.categoryCode() == art::errors::Configuration);
  }
}

DEFINE_ART_MODULE(TestTFileServiceNoRegister)
