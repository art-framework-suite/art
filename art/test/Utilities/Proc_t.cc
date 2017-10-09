#include "art/Utilities/LinuxProcData.h"
#include "art/Utilities/LinuxProcMgr.h"
#include "art/Utilities/ScheduleID.h"
#include "cetlib/SimultaneousFunctionSpawner.h"
#include "cetlib/container_algorithms.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <mutex>
#include <tuple>
#include <vector>

using art::LinuxProcData;
using art::LinuxProcMgr;
using art::ScheduleID;
using vsize_t = LinuxProcData::vsize_t;
using rss_t = LinuxProcData::rss_t;

namespace {

  using sid_size_type = ScheduleID::size_type;
  using logged_memory_t =
    std::vector<std::tuple<sid_size_type, vsize_t, rss_t>>;

  class ProcInfoTester {
  public:
    explicit ProcInfoTester(sid_size_type const nSchedules)
      : procMgr_{nSchedules}
    {}

    void
    update(sid_size_type const sid, logged_memory_t& loggedMemory)
    {
      auto const data = procMgr_.getCurrentData(sid);
      std::lock_guard<decltype(m_)> hold{m_};
      loggedMemory.emplace_back(sid,
                                LinuxProcData::getValueInMB<vsize_t>(data),
                                LinuxProcData::getValueInMB<rss_t>(data));
    }

  private:
    LinuxProcMgr procMgr_;
    static std::mutex m_;
  };

  std::mutex ProcInfoTester::m_{};

  // 6 update calls in test
  void
  test(ProcInfoTester& procInfo, sid_size_type const sid, logged_memory_t& data)
  {
    procInfo.update(sid, data);

    std::vector<int> testVec(400, 0.);
    procInfo.update(sid, data);

    testVec.resize(800, 0.);
    procInfo.update(sid, data);

    testVec.resize(1600, 0.);
    procInfo.update(sid, data);

    std::vector<int> anotherVec(1000000, 0.);
    procInfo.update(sid, data);

    testVec.resize(10000, 0.);
    procInfo.update(sid, data);
  }

} // namespace

int
main()
{
  sid_size_type constexpr nSchedules{2};
  ProcInfoTester procInfo{nSchedules};

  logged_memory_t loggedMemory;
  std::vector<std::function<void()>> tasks;
  for (sid_size_type i{}; i < nSchedules; ++i) {
    tasks.push_back(
      [&procInfo, i, &loggedMemory] { test(procInfo, i, loggedMemory); });
  }

  cet::SimultaneousFunctionSpawner launch{tasks};
  assert(loggedMemory.size() == 12ull);

  logged_memory_t schedule1, schedule2;
  std::partition_copy(loggedMemory.cbegin(),
                      loggedMemory.cend(),
                      std::back_inserter(schedule1),
                      std::back_inserter(schedule2),
                      [](auto const& t) {
                        return std::get<0>(t) == 0; // ScheduleID 0
                      });
  assert(schedule1.size() == schedule2.size());
}
