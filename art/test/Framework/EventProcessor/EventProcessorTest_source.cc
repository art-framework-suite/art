// Our simulated input file format is:
// A parameter in a parameter set, which contains a vector of vector of int.
// Each inner vector is a triplet of run/subrun/event number.
//   -1 means no new item of that type
//

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>
#include <vector>

namespace {
  std::uint32_t number(std::string const& action)
  {
    if (std::count(action.begin(), action.end(), ':') != 1u) {
      throw art::Exception{art::errors::Configuration}
      << "The specified action for \"" << action << "\" must\n"
      << "contain only one ':'.";
    }
    auto const level = action.substr(0,1);
    auto const symbol = action.substr(2);
    if (symbol.empty()) {
      throw art::Exception{art::errors::Configuration}
      << "The symbol for \"" << action << "\" is empty.\n"
      << "Please provide a positive number, or the character 'f'.\n";
    }

    // For flush values -- r:f, s:f, or e:f
    if (std::isalpha(symbol[0])) {
      if (symbol[0] != 'f') {
        throw art::Exception{art::errors::Configuration}
        << "The character specified for a symbol must be 'f'.\n";
      }
      switch (level[0]) {
      case 'r': return art::IDNumber<art::Level::Run>::flush_value();
      case 's': return art::IDNumber<art::Level::SubRun>::flush_value();
      case 'e': return art::IDNumber<art::Level::Event>::flush_value();
      default:
        throw art::Exception{art::errors::Configuration}
        << "Action specifying flush value does not correspond to 'r', 's', or 'e'.\n";
      }
    }

    // For everything else
    return std::stoul(action.substr(2)); // (e.g.) e:14 - start at "14"
  }

  auto nullTimestamp() { return art::Timestamp{}; }

}

namespace art {
  class MasterProducerRegistry;
}

namespace arttest {

  class EventProcessorTest : public art::InputSource {
  public:

    EventProcessorTest(fhicl::ParameterSet const& ps,
                       art::InputSourceDescription& isd)
      : isd_{isd}
      , fileNames_{ps.get<std::vector<std::string>>("fileNames")}
    {}


    std::unique_ptr<art::FileBlock> readFile(art::MasterProductRegistry&) override
    {
      inputFile_.open(currentName_);
      return std::make_unique<art::FileBlock>(art::FileFormatVersion{1,"EventProcessorTest_2017a"}, currentName_);
    }

    void closeFile() override
    {
      inputFile_.close();
    }

    art::input::ItemType nextItemType() override
    {
      art::input::ItemType rc {art::input::IsStop};
      std::string action {};
      if (std::getline(inputFile_, action)) {
        if (action[0] == 'r') {
          auto const r = number(action);
          run_ = (r == art::IDNumber<art::Level::Run>::flush_value()) ? art::RunID::flushRun() : art::RunID{r};
          rc = art::input::IsRun;
        }
        else if (action[0] == 's') {
          auto const sr = number(action);
          subRun_ = (sr == art::IDNumber<art::Level::SubRun>::flush_value()) ? art::SubRunID::flushSubRun() : art::SubRunID{run_, sr};
          rc = art::input::IsSubRun;
        }
        else if (action[0] == 'e') {
          auto const e = number(action);
          event_ = (e == art::IDNumber<art::Level::Event>::flush_value()) ? art::EventID::flushEvent() : art::EventID{subRun_, e};
          // a special value for test purposes only
          if (event_.event() != 7) {
            rc = art::input::IsEvent;
          }
        }
        else {
          throw art::Exception{art::errors::Configuration}
          << "Test pattern \"" << action << "\" not recognized.";
        }
      }
      else if (!fileNames_.empty()) {
        currentName_ = fileNames_.front();
        fileNames_.erase(std::cbegin(fileNames_));
        return art::input::IsFile;
      }
      return rc;
    }

    art::RunID run() const override { return run_; }
    art::SubRunID subRun() const override { return subRun_; }

    std::unique_ptr<art::RunPrincipal> readRun() override
    {
      art::RunAuxiliary const aux {run_, nullTimestamp(), nullTimestamp()};
      return std::make_unique<art::RunPrincipal>(aux, isd_.moduleDescription.processConfiguration());
    }

    std::unique_ptr<art::SubRunPrincipal> readSubRun(cet::exempt_ptr<art::RunPrincipal> rp) override
    {
      art::SubRunAuxiliary const aux {subRun_, nullTimestamp(), nullTimestamp()};
      auto srp = std::make_unique<art::SubRunPrincipal>(aux, isd_.moduleDescription.processConfiguration());
      srp->setRunPrincipal(rp);
      return std::move(srp);
    }

    using art::InputSource::readEvent;
    std::unique_ptr<art::EventPrincipal> readEvent(cet::exempt_ptr<art::SubRunPrincipal> srp) override
    {
      art::EventAuxiliary const aux {event_, nullTimestamp(), true};
      auto ep = std::make_unique<art::EventPrincipal>(aux, isd_.moduleDescription.processConfiguration());
      ep->setSubRunPrincipal(srp);
      return std::move(ep);
    }

    std::unique_ptr<art::RangeSetHandler> runRangeSetHandler() override
    {
      return std::make_unique<art::OpenRangeSetHandler>(run_.run());
    }

    std::unique_ptr<art::RangeSetHandler> subRunRangeSetHandler() override
    {
      return std::make_unique<art::OpenRangeSetHandler>(subRun_.run());
    }


  private:
    art::InputSourceDescription const isd_;
    std::ifstream inputFile_ {};
    std::string currentName_ {};
    std::vector<std::string> fileNames_{};
    art::RunID run_ {};
    art::SubRunID subRun_ {};
    art::EventID event_ {};
  };

}

DEFINE_ART_INPUT_SOURCE(arttest::EventProcessorTest)
