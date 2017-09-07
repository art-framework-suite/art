#define BOOST_TEST_MODULE ( ProcessHistoryRegistry_t )
#include "cetlib/quiet_unit_test.hpp"

#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "cetlib/SimultaneousFunctionSpawner.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/test_macros.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

using namespace art;
using namespace std::string_literals;
using fhicl::ParameterSet;

namespace {
  ProcessConfiguration makeProcessConfiguration(std::string const& process_name,
                                                std::string const& module_label)
  {
    ParameterSet moduleParams;
    moduleParams.put("module_type", "IntProducer"s);
    moduleParams.put("module_label", module_label);

    ParameterSet processParams;
    processParams.put("process_name", process_name);
    processParams.put(module_label, moduleParams);

    return ProcessConfiguration{process_name, processParams.id(), getReleaseVersion()};
  }
}

BOOST_AUTO_TEST_SUITE(ProcessHistoryTest)

BOOST_AUTO_TEST_CASE(concurrent_insertion_reading)
{
  std::vector<ProcessConfiguration> pcs;
  pcs.push_back(makeProcessConfiguration("p1", "m1"));
  pcs.push_back(makeProcessConfiguration("p2", "m2"));
  pcs.push_back(makeProcessConfiguration("pA", "mA"));
  pcs.push_back(makeProcessConfiguration("pB", "mB"));

  auto makeProcessHistory = [&pcs](std::size_t const first, std::size_t const last) {
    ProcessHistory ph;
    ph.push_back(pcs.at(first));
    ph.push_back(pcs.at(last));
    return ph;
  };

  std::vector<ProcessHistory> histories;
  histories.push_back(makeProcessHistory(0,1));
  histories.push_back(makeProcessHistory(0,3));
  histories.push_back(makeProcessHistory(2,1));
  histories.push_back(makeProcessHistory(2,3));

  // Insert histories in parallel
  {
    std::vector<std::function<void()>> tasks;
    cet::transform_all(histories, std::back_inserter(tasks),
                       [](auto const& h) {
                         return [&h]{ ProcessHistoryRegistry::emplace(h.id(), h); };
                       });
    cet::SimultaneousFunctionSpawner sfs {tasks};
  }

  BOOST_REQUIRE_EQUAL(ProcessHistoryRegistry::get().size(), histories.size());

  // Retrieve histories in parallel
  {
    std::vector<ProcessHistory> retrievedHistories (histories.size());
    std::vector<std::function<void()>> tasks;
    cet::for_all_with_index(histories,
                            [&retrievedHistories, &tasks](std::size_t const i, auto const& h) {
                              auto& entry = retrievedHistories[i];
                              tasks.push_back([&h, &entry]{
                                  ProcessHistory retrievedHistory;
                                  ProcessHistoryRegistry::get(h.id(), retrievedHistory);
                                  entry = std::move(retrievedHistory);
                                });
                            });
    cet::SimultaneousFunctionSpawner sfs {tasks};
    //CET_CHECK_EQUAL_COLLECTIONS(histories, retrievedHistories);
  }
}

BOOST_AUTO_TEST_SUITE_END()
