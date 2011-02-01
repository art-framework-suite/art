#include "test/Framework/Services/Message/UnitTestClient_SSubRun.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Utilities/Exception.h"

#include <iostream>
#include <string>

namespace arttest
{

bool UTC_SL1::enableNotYetCalled = true;
int UTC_SL1::n = 0;
int UTC_SL2::n = 0;

void
  UTC_SL1::analyze( art::Event      const & e
                            , art::EventSetup const & /*unused*/
                              )
{
  if (enableNotYetCalled) {
    art::EnableLoggedErrorsSummary();
    enableNotYetCalled = false;
  }
  n++;
  if (n <= 2) return;
  art::LogError   ("cat_A")   << "S1 with identifier " << identifier
  			      << " n = " << n;
  art::LogError   ("grouped_cat")  << "S1 timer with identifier " << identifier;
}

void
  UTC_SL2::analyze( art::Event      const & e
                            , art::EventSetup const & /*unused*/
                              )
{
  n++;
  if (n <= 2) return;
  art::LogError   ("cat_A")   << "S2 with identifier " << identifier;
  art::LogError   ("grouped_cat") << "S2 timer with identifier " << identifier;
  art::LogError   ("cat_B")   << "S2B with identifier " << identifier;
  for (int i = 0; i<n; ++i) {
    art::LogError   ("cat_B")   << "more S2B";
  }
}

void
  UTC_SLUMMARY::analyze( art::Event      const & e
                            , art::EventSetup const & /*unused*/
                              )
{
  if (!art::FreshErrorsExist()) {
    art::LogInfo   ("NoFreshErrors") << "Not in this event, anyway";
  }
  std::vector<art::ErrorSummaryEntry> es = art::LoggedErrorsSummary();
  std::ostringstream os;
  for (unsigned int i = 0; i != es.size(); ++i) {
    os << es[i].category << "   " << es[i].module << "   "
       << es[i].count << "\n";
  }
  art::LogVerbatim ("ErrorsInEvent") << os.str();
}

void
  UTC_SLUMMARY::endSubRun( art::SubRun const & sr
                            , art::EventSetup const & /*unused*/
                              )
{
  // throw artZ::Exception("endSubRun called!");
  art::LogInfo ("endSubRun") << "endSubRun() called";
}


}  // arttest

using arttest::UTC_SL1;
using arttest::UTC_SL2;
using arttest::UTC_SLUMMARY;
DEFINE_FWK_MODULE(UTC_SL1);
DEFINE_FWK_MODULE(UTC_SL2);
DEFINE_FWK_MODULE(UTC_SLUMMARY);
