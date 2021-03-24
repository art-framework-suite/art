// vim: set sw=2 expandtab :
#define BOOST_TEST_MODULE (PathManager Test)
#include "boost/test/unit_test.hpp"

#include "art/Framework/Art/detail/prune_configuration.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/UpdateOutputCallbacks.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/intermediate_table.h"

#include <tuple>
#include <vector>

using namespace std;
using namespace art;

struct PathManagerTestFixture {
  ActionTable atable;
  UpdateOutputCallbacks preg;
  ProductDescriptions productsToProduce;
  ActivityRegistry areg;
};

BOOST_FIXTURE_TEST_SUITE(PathManager_t, PathManagerTestFixture)

BOOST_AUTO_TEST_CASE(Construct)
{
  vector<tuple<string, errors::ErrorCodes, string>> test_sets;

  // Empty.
  test_sets.emplace_back("", static_cast<errors::ErrorCodes>(0), "");

  // Correct.
  test_sets.emplace_back("process_name: \"test\" "
                         "physics: { "
                         "  producers: { "
                         "    p: { module_type: PMTestProducer } "
                         "  } "
                         " p1: [ p ] "
                         "}",
                         static_cast<errors::ErrorCodes>(0),
                         "");

  // Correct.  Empty path-selection overrides despite presence of paths p1 and
  // e1.
  test_sets.emplace_back("process_name: \"test\" "
                         "physics: {"
                         "  producers: {"
                         "    p: { module_type: PMTestProducer }"
                         "  }"
                         "  analyzers: {"
                         "    a: { module_type: DummyAnalyzer }"
                         "  }"
                         "  p1: [p]"
                         "  e1: [a]"
                         "  trigger_paths: []"
                         "  end_paths: []"
                         "}",
                         static_cast<errors::ErrorCodes>(0),
                         "");

  // Module type mismatch.
  test_sets.emplace_back("process_name: \"test\" "
                         "physics: { "
                         "  producers: { "
                         "    p: { module_type: PMTestFilter } "
                         "  } "
                         " p1: [ p ] "
                         "}",
                         errors::Configuration,
                         "---- Configuration BEGIN\n"
                         "  The following were encountered while processing "
                         "the module configurations:\n"
                         "    ERROR: Module with label p of type PMTestFilter "
                         "is configured as a producer but defined in code as a "
                         "filter.\n"
                         "---- Configuration END\n");

  for (auto const& [config_string, error_code, error_msg] : test_sets) {
    auto raw_config = fhicl::parse_document(config_string);
    auto const enabled_modules =
      detail::prune_config_if_enabled(false, true, raw_config);
    try {
      auto const ps = fhicl::ParameterSet::make(raw_config);
      PathManager pm(
        ps, preg, productsToProduce, atable, areg, enabled_modules);
      assert(error_code == 0);
    }
    catch (Exception const& e) {
      if ((e.categoryCode() != error_code) || (e.what() != error_msg)) {
        throw;
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
