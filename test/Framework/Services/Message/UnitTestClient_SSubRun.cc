#include "test/Framework/Services/Message/UnitTestClient_SSubRun.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "cetlib/exception.h"

#include <iostream>
#include <string>

namespace arttest {

  bool UTC_SL1::enableNotYetCalled = true;
  int UTC_SL1::n = 0;
  int UTC_SL2::n = 0;

  void
  UTC_SL1::analyze(art::Event      const & e
                   , art::EventSetup const & /*unused*/
                  )
  {
    if (enableNotYetCalled) {
      art::EnableLoggedErrorsSummary();
      enableNotYetCalled = false;
    }
    n++;
    if (n <= 2) { return; }
    mf::LogError("cat_A")   << "S1 with identifier " << identifier
                            << " n = " << n;
    mf::LogError("grouped_cat")  << "S1 timer with identifier " << identifier;
  }

  void
  UTC_SL2::analyze(art::Event      const & e
                   , art::EventSetup const & /*unused*/
                  )
  {
    n++;
    if (n <= 2) { return; }
    mf::LogError("cat_A")   << "S2 with identifier " << identifier;
    mf::LogError("grouped_cat") << "S2 timer with identifier " << identifier;
    mf::LogError("cat_B")   << "S2B with identifier " << identifier;
    for (int i = 0; i < n; ++i) {
      mf::LogError("cat_B")   << "more S2B";
    }
  }

  void
  UTC_SLUMMARY::analyze(art::Event      const & e
                        , art::EventSetup const & /*unused*/
                       )
  {
    if (!art::FreshErrorsExist()) {
      mf::LogInfo("NoFreshErrors") << "Not in this event, anyway";
    }
    std::vector<art::ErrorSummaryEntry> es = mf::LoggedErrorsSummary();
    std::ostringstream os;
    for (unsigned int i = 0; i != es.size(); ++i) {
      os << es[i].category << "   " << es[i].module << "   "
         << es[i].count << "\n";
    }
    mf::LogVerbatim("ErrorsInEvent") << os.str();
  }

  void
  UTC_SLUMMARY::endSubRun(art::SubRun const & sr
                          , art::EventSetup const & /*unused*/
                         )
  {
    // throw cet::exception("endSubRun called!");
    mf::LogInfo("endSubRun") << "endSubRun() called";
  }


}  // arttest

using arttest::UTC_SL1;
using arttest::UTC_SL2;
using arttest::UTC_SLUMMARY;
DEFINE_ART_MODULE(UTC_SL1);
DEFINE_ART_MODULE(UTC_SL2);
DEFINE_ART_MODULE(UTC_SLUMMARY);
