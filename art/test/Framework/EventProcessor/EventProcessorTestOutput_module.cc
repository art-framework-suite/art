// ======================================================================
//
// EventProcessorTestOutput: Allows testing the output-file switching
// mechanism. See OUTPUT_COMMENT macro below.
//
// ======================================================================

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Utilities/ConfigurationTable.h"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/TupleAs.h"

using namespace art;

namespace {
  class SwitchPoint {
  public:

    explicit SwitchPoint() = default;

    explicit SwitchPoint(std::string const& filename, std::vector<std::uint32_t> const& idNumbers)
      : filename_{filename}
    {
      if (filename_.empty()) {
        throw Exception {errors::Configuration} << "Cannot create a valid SwitchPoint object with an empty filename.";
      }

      if (idNumbers.empty()) {
        return;
      }

      switch (idNumbers.size()) {
        // N.B. Rely on fall-through behavior to set all relevant data.
      case 3ull: e_ = idNumbers[2];
      case 2ull: sr_ = idNumbers[1];
      case 1ull: r_ = idNumbers[0];
        break;
      default:
        throw Exception{errors::Configuration}
        << "Specified too many id numbers.";
      }
    }

    bool valid() const { return filename_ != ""; } // Very weak check, but probably sufficient for this test.

    bool matches(std::string const& fn) const
    {
      return filename_ == fn;
    }

    bool matches(std::string const& fn, RunID const& id) const
    {
      return matches(fn) && r_ == id.run();
    }

    bool matches(std::string const& fn, SubRunID const& id) const
    {
      return matches(fn, id.runID()) && sr_ == id.subRun();
    }

    bool matches(std::string const& fn, EventID const& id) const
    {
      return matches(fn, id.subRunID()) && e_ == id.event();
    }

  private:
    std::string filename_ {};
    RunNumber_t r_ {IDNumber<Level::Run>::invalid()};
    SubRunNumber_t sr_ {IDNumber<Level::SubRun>::invalid()};
    EventNumber_t e_ {IDNumber<Level::Event>::invalid()};
  };
}

#define OUTPUT_COMMENT                                                \
 "To indicate where an output file should switch, one can\n"         \
 "specify the switch points via:\n\n"                                \
 "  switchAfter: [\n"                                                \
 "    [\"a.txt\", [1]    ], # ==> Switch after Run 1 in file \"a.txt\"\n" \
 "    [\"a.txt\", []     ], # ==> Switch after file \"a.txt\"\n"         \
 "    [\"b.txt\", [1,4]  ], # ==> Switch after Run 1, SubRun 4 in file \"b.txt\"\n" \
 "    [\"b.txt\", [2,3,7]]  # ==> Switch after Run 2, SubRun 3, Event 7 in file \"b.txt\"\n" \
 "  ]\n\n"                                                           \
 "Note that the switching behavior should be put in the order that\n" \
 "the switching is expected--e.g. it would be an error to\n"         \
 "specify [\"a.txt\", []] as the first item since that would\n"      \
 "tell the module to switch after the first file has been\n"         \
 "processed, and then switch after Run 1 in file \"a.txt\". In\n"    \
 "other words, no sorting is done of the specified switch points."

namespace arttest {

  class EventProcessorTestOutput final : public OutputModule {
  public:

    struct Config {
      fhicl::TableFragment<OutputModule::Config> omConfig;
      fhicl::Sequence<fhicl::TupleAs<SwitchPoint(std::string,fhicl::Sequence<std::uint32_t>)>> switchAfter {
        fhicl::Name("switchAfter"), fhicl::Comment(OUTPUT_COMMENT), std::vector<SwitchPoint>{}};
    };

    using Parameters = WrappedTable<Config, OutputModule::Config::KeysToIgnore>;
    explicit EventProcessorTestOutput(Parameters const& ps)
      : OutputModule{ps().omConfig, ps.get_PSet()}
      , switchPoints_{ps().switchAfter()}
    {
      if (!switchPoints_.empty()) {
        activeSwitchPoint_ = switchPoints_.front();
      }
    }

  private:

    void respondToOpenInputFile(FileBlock const& fb)
    {
      //      std::cout << __func__ << '\n';
      currentInputFileName_ = fb.fileName();
    }

    void write(EventPrincipal& ep) override
    {
      //      std::cout << __func__ << '\n';
      requestsFileClose_ = activeSwitchPoint_.matches(currentInputFileName_, ep.id());
      if (requestsFileClose_) {
        updateSwitchPoints();
      }
    }

    void writeSubRun(SubRunPrincipal& srp) override
    {
      //      std::cout << __func__ << '\n';
      requestsFileClose_ = activeSwitchPoint_.matches(currentInputFileName_, srp.id());
      if (requestsFileClose_) {
        updateSwitchPoints();
      }
    }

    void writeRun(RunPrincipal& rp) override
    {
      //      std::cout << __func__ << '\n';
      requestsFileClose_ = activeSwitchPoint_.matches(currentInputFileName_, rp.id());
      if (requestsFileClose_) {
        updateSwitchPoints();
      }
    }

    void incrementInputFileNumber() override
    {
      //      std::cout << __func__ << '\n';
      requestsFileClose_ = activeSwitchPoint_.matches(currentInputFileName_);
      if (requestsFileClose_) {
        updateSwitchPoints();
      }
    }

    bool requestsToCloseFile() const override
    {
      //      std::cout << __func__ << ": " << std::boolalpha << requestsFileClose_ << '\n';
      return requestsFileClose_;
    }

    Boundary fileSwitchBoundary() const { return Boundary::Event; }

    void updateSwitchPoints()
    {
      //      std::cout << __func__ << '\n';
      assert(activeSwitchPoint_.valid());
      if (switchPoints_.empty()) {
        // Invalidate the next switch point--matching against a
        // default-constructed SwitchPoint will always return false.
        activeSwitchPoint_ = SwitchPoint{};
      }
      else {
        // Pop the front
        switchPoints_.erase(std::cbegin(switchPoints_));
        activeSwitchPoint_ = switchPoints_.front();
      }
    }

    std::vector<SwitchPoint> switchPoints_;
    SwitchPoint activeSwitchPoint_ {};
    std::string currentInputFileName_ {};
    bool requestsFileClose_ {false};
  };

}
#undef OUTPUT_COMMENT
DEFINE_ART_MODULE(arttest::EventProcessorTestOutput)
