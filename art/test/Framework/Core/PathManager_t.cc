// vim: set sw=2 expandtab :
#define BOOST_TEST_MODULE (PathManager Test)
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Art/detail/prune_configuration.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/UpdateOutputCallbacks.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"

#include <tuple>
#include <vector>

using namespace std;
using namespace art;

struct PathManagerTestFixture {
  ActionTable atable;
  UpdateOutputCallbacks preg;
  ProductDescriptions productsToProduce;
  ActivityRegistry areg;
  std::map<std::string, detail::ModuleKeyAndType> enabledModules;
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

  // Duplicate label.
  test_sets.emplace_back(
    "process_name: \"test\" "
    "physics: { "
    "  analyzers: { "
    "    p: { module_type: PMTestAnalyzer } "
    "  } "
    "  filters: { "
    "    p: { module_type: PMTestFilter } "
    "  } "
    " p1: [ p ] "
    "}",
    errors::Configuration,
    "---- Configuration BEGIN\n"
    "  An error was encountered while processing module configurations.\n"
    "  Module label 'p' has been used in 'physics.analyzers' and "
    "'physics.filters'.\n"
    "  Module labels must be unique across an art process.\n"
    "---- Configuration END\n");

  // Inhomogeneous path.
  test_sets.emplace_back(
    "process_name: \"test\" "
    "physics: { "
    "  analyzers: { "
    "    a: { module_type: PMTestAnalyzer } "
    "  } "
    "  filters: { "
    "    f: { module_type: PMTestFilter } "
    "  } "
    "  producers: { "
    "    p: { module_type: PMTestProducer } "
    "  } "
    " p1: [ f, p, a ] "
    "}",
    errors::Configuration,
    "---- Configuration BEGIN\n"
    "  An error was encountered while processing a path configuration.\n"
    "  The following entries in path p1 are observers when all other\n"
    "  entries are modifiers:\n"
    "    'a'\n"
    "---- Configuration END\n");

  // Unconfigured label.
  test_sets.emplace_back(
    "process_name: \"test\" "
    "physics: { "
    " p1: [ \"-f\", p, a ] "
    "}",
    errors::Configuration,
    "---- Configuration BEGIN\n"
    "  The following error was encountered while processing a path "
    "configuration:\n"
    "  Entry f in path p1 does not have a module configuration.\n"
    "---- Configuration END\n");

  // Incorrectly included parameter in "physics" block
  test_sets.emplace_back(
    "process_name: pathMisspecification "
    "physics: { "
    "  producers : {} "
    "  filters   : {} "
    "  analyzers : {} "
    "test : atom "
    "check : { "
    "  cannot : put "
    "  random : table } }",
    errors::Configuration,
    "---- Configuration BEGIN\n"
    "  \n"
    "  You have specified the following unsupported parameters in the\n"
    "  \"physics\" block of your configuration:\n"
    "  \n"
    "     \"physics.check\"   (table)\n"
    "     \"physics.test\"   (atom)\n"
    "  \n"
    "  Supported parameters include the following tables:\n"
    "     \"physics.producers\"\n"
    "     \"physics.filters\"\n"
    "     \"physics.analyzers\"\n"
    "  and sequences. Atomic configuration parameters are not allowed.\n"
    "  \n"
    "---- Configuration END\n");

  // Incorrectly included end path as trigger path
  test_sets.emplace_back("process_name: MisspecifiedEndPath\n"
                         "physics.analyzers.a1: {\n"
                         "   module_type: DummyAnalyzer\n"
                         "}\n"
                         "physics.e1: [a1]\n"
                         "physics.trigger_paths: [e1]\n",
                         errors::Configuration,
                         "---- Configuration BEGIN\n"
                         "  Path configuration: The following were encountered "
                         "while processing path configurations:\n"
                         "    ERROR: Path 'e1' is configured as a trigger path "
                         "but is actually an end path.\n"
                         "---- Configuration END\n");

  // Incorrectly included trigger path as end path
  test_sets.emplace_back(
    "process_name: MisspecifiedTriggerPath\n"
    "physics.producers.d1: {\n"
    "   module_type: \"art/test/Framework/Art/PrintAvailable/DummyProducer\"\n"
    "}\n"
    "physics.p1: [d1]\n"
    "physics.trigger_paths: [p1]\n",
    errors::Configuration,
    "---- Configuration BEGIN\n"
    "  Path configuration: The following were encountered while processing "
    "path configurations:\n"
    "    ERROR: Path 'p1' is configured as an end path but is actually a "
    "trigger path.\n"
    "---- Configuration END\n");

  // Incorrectly applied '!' and '-' characters to modules that are not filters.
  test_sets.emplace_back(
    "process_name: MisspecifiedModuleLabel\n"
    "physics.producers.d1: {\n"
    "   module_type: \"art/test/Framework/Art/PrintAvailable/DummyProducer\"\n"
    "}\n"
    "physics.analyzers.d2: {\n"
    "   module_type: \"art/test/Framework/Art/PrintAvailable/DummyAnalyzer\"\n"
    "}\n"
    "physics.p1: [\"!d1\"]\n"
    "physics.e1: [\"-d2\"]\n",
    errors::Configuration,
    "---- Configuration BEGIN\n"
    "  Path configuration: The following were encountered while processing "
    "path configurations:\n"
    "    ERROR: Module d2 in path e1 is an analyzer and cannot have a '!' or "
    "'-' prefix.\n"
    "    ERROR: Module d1 in path p1 is a producer and cannot have a '!' or "
    "'-' prefix.\n"
    "---- Configuration END\n");

  for (auto const& [config_string, error_code, error_msg] : test_sets) {
    fhicl::intermediate_table raw_config;
    parse_document(config_string, raw_config);
    try {
      auto const enabled_modules =
        detail::prune_config_if_enabled(false, raw_config);
      fhicl::ParameterSet ps;
      make_ParameterSet(raw_config, ps);
      PathManager pm(ps, preg, productsToProduce, atable, areg, enabledModules);
    }
    catch (Exception const& e) {
      if ((e.categoryCode() != error_code) || (e.what() != error_msg)) {
        throw;
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
