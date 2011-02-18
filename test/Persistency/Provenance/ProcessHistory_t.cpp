#include <cassert>
#include <string>
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "fhiclcpp/ParameterSetID.h"
#include "art/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"

int main()
{
  art::ProcessHistory pnl1;
  assert(pnl1 == pnl1);
  art::ProcessHistory pnl2;
  assert(pnl1 == pnl2);
  art::ProcessConfiguration iHLT(std::string("HLT"), fhicl::ParameterSetID(), art::getReleaseVersion(), art::getPassID());
  art::ProcessConfiguration iRECO(std::string("RECO"), fhicl::ParameterSetID(), art::getReleaseVersion(), art::getPassID());
  pnl2.push_back(iHLT);
  assert(pnl1 != pnl2);
  art::ProcessHistory pnl3;
  pnl3.push_back(iHLT);
  pnl3.push_back(iRECO);

  art::ProcessHistoryID id1 = pnl1.id();
  art::ProcessHistoryID id2 = pnl2.id();
  art::ProcessHistoryID id3 = pnl3.id();

  assert(id1 != id2);
  assert(id2 != id3);
  assert(id3 != id1);

  art::ProcessHistory pnl4;
  pnl4.push_back(iHLT);
  art::ProcessHistoryID id4 = pnl4.id();
  assert(pnl4 == pnl2);
  assert (id4 == id2);

  art::ProcessHistory pnl5;
  pnl5 = pnl3;
  assert(pnl5 == pnl3);
  assert(pnl5.id() == pnl3.id());
}
