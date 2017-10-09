#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "cetlib/quiet_unit_test.hpp"
#include "fhiclcpp/types/TupleAs.h"

#include <string>

namespace {

  struct TagBuilder {
    TagBuilder(std::string const& label, std::string const& process)
      : label_{label}, process_{process}
    {}

    art::InputTag
    operator()(std::string const& instance)
    {
      return {label_ + ":" + instance + ":" + process_};
    }

    std::string label_;
    std::string process_;
  };

  struct Config {
    fhicl::TupleAs<TagBuilder(std::string, std::string)> tagInfo{
      fhicl::Name("tagInfo"),
      fhicl::Comment("First argument is the module label.\n"
                     "Second argument is the process name")};
  };
}

namespace arttest {

  class ImplicitRSAssignmentAnalyzer : public art::EDAnalyzer {
    TagBuilder tagInfo_;

  public:
    using Parameters = EDAnalyzer::Table<Config>;
    explicit ImplicitRSAssignmentAnalyzer(Parameters const& config)
      : art::EDAnalyzer{config}, tagInfo_{config().tagInfo()}
    {}

    void
    beginRun(art::Run const& r) override
    {
      auto const& numH = r.getValidHandle<unsigned>(tagInfo_("bgnRunNum"));
      auto const& denomH = r.getValidHandle<unsigned>(tagInfo_("bgnRunDenom"));
      BOOST_CHECK(art::same_ranges(numH, denomH));
      BOOST_CHECK(numH.provenance()->rangeOfValidity().is_full_run());
    }

    void
    beginSubRun(art::SubRun const& sr) override
    {
      auto const& numH = sr.getValidHandle<unsigned>(tagInfo_("bgnSubRunNum"));
      auto const& denomH =
        sr.getValidHandle<unsigned>(tagInfo_("bgnSubRunDenom"));
      BOOST_CHECK(art::same_ranges(numH, denomH));
      BOOST_CHECK(numH.provenance()->rangeOfValidity().is_full_subRun());
    }

    void
    analyze(art::Event const&) override
    {}

    void
    endSubRun(art::SubRun const& sr) override
    {
      auto const& numH = sr.getValidHandle<unsigned>(tagInfo_("endSubRunNum"));
      auto const& denomH =
        sr.getValidHandle<unsigned>(tagInfo_("endSubRunDenom"));
      BOOST_CHECK(art::same_ranges(numH, denomH));
      BOOST_CHECK(!numH.provenance()->rangeOfValidity().is_full_subRun());
    }

    void
    endRun(art::Run const& r) override
    {
      auto const& numH = r.getValidHandle<unsigned>(tagInfo_("endRunNum"));
      auto const& denomH = r.getValidHandle<unsigned>(tagInfo_("endRunDenom"));
      BOOST_CHECK(art::same_ranges(numH, denomH));
      BOOST_CHECK(!numH.provenance()->rangeOfValidity().is_full_run());
    }

  }; // ImplicitRSAssignmentAnalyzer
}

DEFINE_ART_MODULE(arttest::ImplicitRSAssignmentAnalyzer)
