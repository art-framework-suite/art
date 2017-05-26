#include "art/Framework/EventProcessor/detail/memoryReport.h"
#include "art/Utilities/LinuxProcMgr.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

using mf::LogAbsolute;

void art::detail::memoryReport()
{
  LinuxProcMgr procInfo{1};
  LogAbsolute("ArtSummary") << "MemReport  " << "---------- Memory  Summary ---[base-10 MB]----";
  LogAbsolute("ArtSummary") << "MemReport  VmPeak = " << procInfo.getVmPeak() << " VmHWM = " << procInfo.getVmHWM();
  LogAbsolute("ArtSummary") << "";
}
