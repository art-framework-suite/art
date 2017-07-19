#include "art/Framework/EventProcessor/detail/memoryReport.h"
#include "art/Utilities/LinuxProcMgr.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

using mf::LogPrint;

void art::detail::memoryReport()
{
  LinuxProcMgr procInfo{1};
  LogPrint("ArtSummary") << "MemReport  " << "---------- Memory  Summary ---[base-10 MB]----";
  LogPrint("ArtSummary") << "MemReport  VmPeak = " << procInfo.getVmPeak() << " VmHWM = " << procInfo.getVmHWM();
  LogPrint("ArtSummary") << "";
}
