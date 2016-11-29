#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "cetlib/quiet_unit_test.hpp"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace fhicl { class ParameterSet; }

using std::make_unique;

namespace {

  bool insert_failure(art::Exception const& e)
  {
    return e.categoryCode() == art::errors::ProductPutFailure;
  }

  bool run_fragment_put(std::string const what)
  {
    return what.find("  Cannot put a product corresponding to a full Run using\n"
                     "  art::runFragment().  This can happen if you attempted to\n"
                     "  put a product at beginRun using art::runFragment().\n"
                     "  Please use either:\n"
                     "     art::fullRun(), or\n"
                     "     art::runFragment(art::RangeSet const&)\n"
                     "  or contact artists@fnal.gov for assistance.\n") != std::string::npos;
  }

  bool run_fragment_full_run_put(std::string const what)
  {
    return what.find("  Cannot put a product corresponding to a full Run using\n"
                     "  art::runFragment(art::RangeSet&).  Please use:\n"
                     "     art::fullRun()\n"
                     "  or contact artists@fnal.gov for assistance.\n") != std::string::npos;
  }

  bool subRun_fragment_put(std::string const what)
  {
    return what.find("  Cannot put a product corresponding to a full SubRun using\n"
                     "  art::subRunFragment().  This can happen if you attempted to\n"
                     "  put a product at beginSubRun using art::subRunFragment().\n"
                     "  Please use either:\n"
                     "     art::fullSubRun(), or\n"
                     "     art::subRunFragment(art::RangeSet const&)\n"
                     "  or contact artists@fnal.gov for assistance.\n") != std::string::npos;
  }

  bool subRun_fragment_full_subRun_put(std::string const what)
  {
    return what.find("  Cannot put a product corresponding to a full SubRun using\n"
                     "  art::subRunFragment(art::RangeSet&).  Please use:\n"
                     "     art::fullSubRun()\n"
                     "  or contact artists@fnal.gov for assistance.\n") != std::string::npos;
  }

  struct Config {};

}

namespace arttest {

  class ImplicitRSAssigmentProducer : public art::EDProducer {
  public:

    using Parameters = art::EDProducer::Table<Config>;
    explicit ImplicitRSAssigmentProducer(Parameters const&)
    {
      produces<unsigned,art::InRun>("bgnRunNum");
      produces<unsigned,art::InRun>("bgnRunDenom");
      produces<unsigned,art::InRun>("bgnRunGarbage1");
      produces<unsigned,art::InRun>("bgnRunGarbage2");
      produces<unsigned,art::InRun>("endRunNum");
      produces<unsigned,art::InRun>("endRunDenom");

      produces<unsigned,art::InSubRun>("bgnSubRunNum");
      produces<unsigned,art::InSubRun>("bgnSubRunDenom");
      produces<unsigned,art::InSubRun>("bgnSubRunGarbage1");
      produces<unsigned,art::InSubRun>("bgnSubRunGarbage2");
      produces<unsigned,art::InSubRun>("endSubRunNum");
      produces<unsigned,art::InSubRun>("endSubRunDenom");
    }

    void beginRun(art::Run& r) override
    {
      r.put(make_unique<unsigned>(2), "bgnRunNum");
      r.put(make_unique<unsigned>(2), "bgnRunDenom", art::fullRun());

      BOOST_CHECK_EXCEPTION(r.put(make_unique<unsigned>(4),
                                  "bgnRunGarbage1",
                                  art::runFragment()),
                            art::Exception,
                            [](art::Exception const& e) {
                              return insert_failure(e) && run_fragment_put(e.what());
                            });


      BOOST_CHECK_EXCEPTION(r.put(make_unique<unsigned>(4),
                                  "bgnRunGarbage2",
                                  art::runFragment(art::RangeSet::forRun(r.id()))),
                            art::Exception,
                            [](art::Exception const& e) {
                              return insert_failure(e) && run_fragment_full_run_put(e.what());
                            });
    }

    void beginSubRun(art::SubRun& sr) override
    {
      sr.put(make_unique<unsigned>(6), "bgnSubRunNum");
      sr.put(make_unique<unsigned>(6), "bgnSubRunDenom", art::fullSubRun());

      BOOST_CHECK_EXCEPTION(sr.put(make_unique<unsigned>(8),
                                   "bgnSubRunGarbage1",
                                   art::subRunFragment()),
                            art::Exception,
                            [](art::Exception const& e) {
                              return insert_failure(e) && subRun_fragment_put(e.what());
                            });

      BOOST_CHECK_EXCEPTION(sr.put(make_unique<unsigned>(8),
                                   "bgnSubRunGarbage2",
                                   art::subRunFragment(art::RangeSet::forSubRun(sr.id()))),
                            art::Exception,
                            [](art::Exception const& e) {
                              return insert_failure(e) && subRun_fragment_full_subRun_put(e.what());
                            });
    }

    void produce(art::Event&) override {}

    void endSubRun(art::SubRun& sr) override
    {
      sr.put(make_unique<unsigned>(1), "endSubRunNum");
      sr.put(make_unique<unsigned>(1), "endSubRunDenom", art::subRunFragment());
    }

    void endRun(art::Run& r) override
    {
      r.put(make_unique<unsigned>(3), "endRunNum");
      r.put(make_unique<unsigned>(3), "endRunDenom", art::runFragment());
    }

  };  // ImplicitRSAssigmentProducer

}

DEFINE_ART_MODULE(arttest::ImplicitRSAssigmentProducer)
