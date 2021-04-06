// ==============================================================
// EventProcessorTestSource
//
// The purpose of this source is solely to drive the EventProcessor,
// which calls the input source's 'nextItemType()' override.  Input
// files for this source are of a simple form (e.g.):
//
//   r:1
//   s:2
//   e:1
//   e:2
//
// where a Run item type with an ID number of 1 is started, a SubRun
// item type with an ID number of 2 is started, and Event item types
// with ID numbers of 1 and 2 are then processed.  Special characters
// are also allowed for testing rarer circumstances the framework must
// support:
//
//   r:f // Provide 'flush' value to indicate run closure
//   s:t // Throw an exception during readSubRun
//
// N.B. For the purposes of this input source, an input specification
// of 'r:t' corresponds to assigning a run number value of
// art::IDNumber<art::Level::Run>::flush_value() - 1, which is used to
// determine whether readRun should throw an exception.
// ==============================================================

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <string>
#include <vector>

namespace {
  std::uint32_t
  flush_value(char const level)
  {
    switch (level) {
    case 'r':
      return art::IDNumber<art::Level::Run>::flush_value();
    case 's':
      return art::IDNumber<art::Level::SubRun>::flush_value();
    case 'e':
      return art::IDNumber<art::Level::Event>::flush_value();
    default:
      throw art::Exception{art::errors::Configuration}
        << "Action specifying flush or throw value does not correspond to "
           "'r', 's', "
           "or 'e'.\n";
    }
  }

  std::uint32_t
  throw_value(char const level)
  {
    return flush_value(level) - 1;
  }

  std::uint32_t
  number(std::string const& action)
  {
    if (std::count(action.begin(), action.end(), ':') != 1u) {
      throw art::Exception{art::errors::Configuration}
        << "The specified action for \"" << action << "\" must\n"
        << "contain only one ':'.";
    }
    auto const level = action.substr(0, 1);
    auto const symbol = action.substr(2);
    if (symbol.empty()) {
      throw art::Exception{art::errors::Configuration}
        << "The symbol for \"" << action << "\" is empty.\n"
        << "Please provide a positive number, or the character 'f' or 't'.\n";
    }

    // For flush/throw values -- r:f, s:f, or e:f
    if (std::isalpha(symbol[0])) {
      if (symbol.size() != 1ull) {
        throw art::Exception{art::errors::Configuration}
          << "There must be only one character after the colon in \"" << action
          << "\".\n"
          << "Please either the character 'f' or 't'.\n";
      }
      switch (symbol[0]) {
      case 'f':
        return flush_value(level[0]);
      case 't':
        return throw_value(level[0]);
      default:
        throw art::Exception{art::errors::Configuration}
          << "The character specified for a symbol must be 'f' or 't'.\n";
      }
    }

    // For everything else
    return std::stoul(action.substr(2)); // (e.g.) e:14 - start at "14"
  }

  constexpr auto
  nullTimestamp()
  {
    return art::Timestamp{};
  }

} // namespace

namespace arttest {

  class EventProcessorTestSource : public art::InputSource {
  public:
    EventProcessorTestSource(fhicl::ParameterSet const& ps,
                             art::InputSourceDescription& isd)
      : InputSource{isd.moduleDescription}
      , pc_{isd.moduleDescription.processConfiguration()}
      , fileNames_{ps.get<std::vector<std::string>>("fileNames")}
    {}

    std::unique_ptr<art::FileBlock>
    readFile() override
    {
      inputFile_.open(currentName_);
      return std::make_unique<art::FileBlock>(
        art::FileFormatVersion{1, "EventProcessorTestSource_2017a"},
        currentName_);
    }

    void
    closeFile() override
    {
      inputFile_.close();
    }

    art::input::ItemType
    nextItemType() override
    {
      art::input::ItemType rc{art::input::IsStop};
      std::string action{};
      if (std::getline(inputFile_, action)) {
        if (action[0] == 'r') {
          auto const r = number(action);
          run_ = (r == art::IDNumber<art::Level::Run>::flush_value()) ?
                   art::RunID::flushRun() :
                   art::RunID{r};
          rc = art::input::IsRun;
        } else if (action[0] == 's') {
          auto const sr = number(action);
          subRun_ = (sr == art::IDNumber<art::Level::SubRun>::flush_value()) ?
                      art::SubRunID::flushSubRun() :
                      art::SubRunID{run_, sr};
          rc = art::input::IsSubRun;
        } else if (action[0] == 'e') {
          auto const e = number(action);
          event_ = (e == art::IDNumber<art::Level::Event>::flush_value()) ?
                     art::EventID::flushEvent() :
                     art::EventID{subRun_, e};
          // a special value for test purposes only
          if (event_.event() != 7) {
            rc = art::input::IsEvent;
          }
        } else {
          throw art::Exception{art::errors::Configuration}
            << "Test pattern \"" << action << "\" not recognized.";
        }
      } else if (!fileNames_.empty()) {
        currentName_ = fileNames_.front();
        fileNames_.erase(cbegin(fileNames_));
        return art::input::IsFile;
      }
      return rc;
    }

    std::unique_ptr<art::RunPrincipal>
    readRun() override
    {
      if (run_.run() == throw_value('r')) {
        throw art::Exception{
          art::errors::FileReadError,
          "There was an exception while reading a run from the input file."};
      }

      art::RunAuxiliary const aux{run_, nullTimestamp(), nullTimestamp()};
      auto rp = std::make_unique<art::RunPrincipal>(aux, pc_, nullptr);
      return rp;
    }

    std::unique_ptr<art::SubRunPrincipal>
    readSubRun(cet::exempt_ptr<art::RunPrincipal const> rp) override
    {
      if (subRun_.subRun() == throw_value('s')) {
        throw art::Exception{
          art::errors::FileReadError,
          "There was an exception while reading a subrun from the input file."};
      }

      art::SubRunAuxiliary const aux{subRun_, nullTimestamp(), nullTimestamp()};
      auto srp = std::make_unique<art::SubRunPrincipal>(aux, pc_, nullptr);
      srp->setRunPrincipal(rp);
      return srp;
    }

    std::unique_ptr<art::EventPrincipal>
    readEvent(cet::exempt_ptr<art::SubRunPrincipal const> srp) override
    {
      if (event_.event() == throw_value('e')) {
        throw art::Exception{
          art::errors::FileReadError,
          "There was an exception while reading an event from the input file."};
      }

      art::EventAuxiliary const aux{event_, nullTimestamp(), true};
      auto ep = std::make_unique<art::EventPrincipal>(aux, pc_, nullptr);
      ep->setSubRunPrincipal(srp);
      return ep;
    }

    std::unique_ptr<art::RangeSetHandler>
    runRangeSetHandler() override
    {
      return std::make_unique<art::OpenRangeSetHandler>(run_.run());
    }

    std::unique_ptr<art::RangeSetHandler>
    subRunRangeSetHandler() override
    {
      return std::make_unique<art::OpenRangeSetHandler>(subRun_.run());
    }

  private:
    art::ProcessConfiguration const pc_;
    std::ifstream inputFile_{};
    std::string currentName_{};
    std::vector<std::string> fileNames_{};
    art::RunID run_{};
    art::SubRunID subRun_{};
    art::EventID event_{};
  };

} // namespace arttest

DEFINE_ART_INPUT_SOURCE(arttest::EventProcessorTestSource)
