#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/DelegatedParameter.h"

namespace art {
  namespace test {
    class DummySource;
  }
} // namespace art

class art::test::DummySource : public art::InputSource {
public:
  struct Config {
    fhicl::DelegatedParameter neverUse{
      fhicl::Name{"neverUse"},
      fhicl::Comment{
        "This source is used to test the print-available plugins option.\n"
        "It should NEVER BE USED in real art job.\n"
        "A job that uses this source WILL fail."}};
  };
  using Parameters = fhicl::WrappedTable<Config>;

  DummySource(Parameters const&, art::InputSourceDescription& isd)
    : InputSource{isd.moduleDescription}
  {}

private:
  std::unique_ptr<art::FileBlock>
  readFile() override
  {
    return nullptr;
  }
  void
  closeFile() override
  {}
  art::input::ItemType
  nextItemType() override
  {
    return art::input::IsStop;
  }
  std::unique_ptr<art::RunPrincipal>
  readRun() override
  {
    return nullptr;
  }
  std::unique_ptr<art::SubRunPrincipal>
  readSubRun(cet::exempt_ptr<art::RunPrincipal const>) override
  {
    return nullptr;
  }

  using art::InputSource::readEvent;
  std::unique_ptr<art::EventPrincipal>
  readEvent(cet::exempt_ptr<art::SubRunPrincipal const>) override
  {
    return nullptr;
  }

  std::unique_ptr<art::RangeSetHandler>
  runRangeSetHandler() override
  {
    return nullptr;
  }
  std::unique_ptr<art::RangeSetHandler>
  subRunRangeSetHandler() override
  {
    return nullptr;
  }
};

DEFINE_ART_INPUT_SOURCE(art::test::DummySource)
