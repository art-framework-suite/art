// vim: set sw=2 expandtab :
#define BOOST_TEST_MODULE (prune_config test)
#include "boost/test/unit_test.hpp"

#include "art/Framework/Art/detail/prune_configuration.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"

#include <string>

using namespace std;
using namespace art;

namespace {
  void
  check_configuration(std::string const& config) noexcept(false)
  {
    fhicl::intermediate_table raw_config;
    parse_document(config, raw_config);
    fhicl::ParameterSet ps;
    make_ParameterSet(raw_config, ps);
    detail::prune_config_if_enabled(false, true, raw_config);
  }

  void
  check_exception(std::string const& config, std::string const& error_msg)
  {
    BOOST_CHECK_EXCEPTION(
      check_configuration(config), art::Exception, [&error_msg](auto const& e) {
        return e.categoryCode() == art::errors::Configuration &&
               e.what() == error_msg;
      });
  }
}

BOOST_AUTO_TEST_SUITE(prune_config_t)

BOOST_AUTO_TEST_CASE(empty_config)
{
  check_configuration({});
}

BOOST_AUTO_TEST_CASE(duplicate_label)
{
  std::string const config{"process_name: \"test\" "
                           "physics: { "
                           "  analyzers: { "
                           "    p: { module_type: PMTestAnalyzer } "
                           "  } "
                           "  filters: { "
                           "    p: { module_type: PMTestFilter } "
                           "  } "
                           " p1: [ p ] "
                           "}"};
  std::string const err_msg{
    "---- Configuration BEGIN\n"
    "  An error occurred while processing module configurations.\n"
    "  Module label 'p' has been used in 'physics.analyzers' and "
    "'physics.filters'.\n"
    "  Module labels must be unique across an art process.\n"
    "---- Configuration END\n"};
  check_exception(config, err_msg);
}

BOOST_AUTO_TEST_CASE(inhomogeneous_path)
{
  std::string const config{"process_name: \"test\" "
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
                           "}"};
  std::string const err_msg{
    "---- Configuration BEGIN\n"
    "  An error occurred while processing a path configuration.\n"
    "  The following modules specified in path p1 are observers when all\n"
    "  other modules are modifiers:\n"
    "    'a'\n"
    "---- Configuration END\n"};
  check_exception(config, err_msg);
}

BOOST_AUTO_TEST_CASE(nonexistent_path_1)
{
  std::string const config{"physics.trigger_paths: [x1]"};
  std::string const err_msg{
    "---- Configuration BEGIN\n"
    "  The following error occurred while processing path configurations:\n"
    "  Unknown path x1 has been specified in 'trigger_paths'.\n"
    "---- Configuration END\n"};
  check_exception(config, err_msg);
}

BOOST_AUTO_TEST_CASE(nonexistent_path_2)
{
  std::string const config{"physics.end_paths: [x1, d6]"};
  std::string const err_msg{
    "---- Configuration BEGIN\n"
    "  The following error occurred while processing path configurations:\n"
    "  Unknown path x1 has been specified in 'end_paths'.\n"
    "  Unknown path d6 has been specified in 'end_paths'.\n"
    "---- Configuration END\n"};
  check_exception(config, err_msg);
}

BOOST_AUTO_TEST_CASE(unconfigured_label)
{
  std::string const config{"process_name: \"test\" "
                           "physics: { "
                           " p1: [ \"-f\", p, a ] "
                           "}"};
  std::string const err_msg{
    "---- Configuration BEGIN\n"
    "  The following error occurred while processing a path "
    "configuration:\n"
    "  Entry with name f in path p1 does not have a module configuration.\n"
    "---- Configuration END\n"};
  check_exception(config, err_msg);
}

BOOST_AUTO_TEST_CASE(illegal_label)
{
  std::string const config{"process_name: \"test\" "
                           "physics: {"
                           "  p1: [ \"!f-g\" ]"
                           "}"};
  std::string const err_msg{
    "---- Configuration BEGIN\n"
    "  There was an error parsing the entry \"!f-g\"in a FHiCL sequence.\n"
    "  The '!' or '-' character may appear as only the first character in the "
    "path entry.\n"
    "---- Configuration END\n"};
  check_exception(config, err_msg);
}

BOOST_AUTO_TEST_CASE(illegal_action_for_non_filter)
{
  std::string const config{"process_name: \"test\" "
                           "physics: {"
                           "  producers: { p: {} }"
                           "  p1: [ \"!p\" ]"
                           "}"};
  std::string const err_msg{"---- Configuration BEGIN\n"
                            "  The following error occurred while "
                            "processing a path configuration:\n"
                            "  Entry with name p in path p1 is a producer and "
                            "cannot have a '!' or '-' prefix.\n"
                            "---- Configuration END\n"};
  check_exception(config, err_msg);
}

BOOST_AUTO_TEST_CASE(unconfigured_label_with_trigger_paths)
{
  std::string const config{"process_name: \"test\" "
                           "physics: { "
                           " p1: [ f ] "
                           " trigger_paths: [ p1 ] "
                           "}"};
  std::string const err_msg{
    "---- Configuration BEGIN\n"
    "  The following error occurred while processing a path "
    "configuration:\n"
    "  Entry with name f in path p1 does not have a module configuration.\n"
    "---- Configuration END\n"};
  check_exception(config, err_msg);
}

BOOST_AUTO_TEST_CASE(misspecified_end_path)
{
  // Incorrectly included end path as trigger path
  std::string const config{"process_name: MisspecifiedEndPath\n"
                           "physics.analyzers.a1: {\n"
                           "   module_type: DummyAnalyzer\n"
                           "}\n"
                           "physics.e1: [a1]\n"
                           "physics.trigger_paths: [e1]\n"};
  std::string const err_msg{
    "---- Configuration BEGIN\n"
    "  The following error occurred while processing a path "
    "configuration:\n"
    "  The 'trigger_paths' override parameter contains the path e1, which has "
    "an\n"
    "  analyzer with the name a1.\n"
    "  \n"
    "  Path e1 should instead be included as part of the 'end_paths' "
    "parameter.\n"
    "  Contact artists@fnal.gov for guidance.\n"
    "---- Configuration END\n"};
  check_exception(config, err_msg);
}

BOOST_AUTO_TEST_CASE(misspecified_trigger_path)
{
  // Incorrectly included trigger path as end path
  std::string const config{"process_name: MisspecifiedTriggerPath\n"
                           "physics.producers.d1: {\n"
                           "  module_type: DummyProducer\n"
                           "}\n"
                           "physics.p1: [d1]\n"
                           "physics.end_paths: [p1]\n"};
  std::string const err_msg{
    "---- Configuration BEGIN\n"
    "  The following error occurred while processing a path "
    "configuration:\n"
    "  The 'end_paths' override parameter contains the path p1, which has a\n"
    "  producer with the name d1.\n"
    "  \n"
    "  Path p1 should instead be included as part of the 'trigger_paths' "
    "parameter.\n"
    "  Contact artists@fnal.gov for guidance.\n"
    "---- Configuration END\n"};
  check_exception(config, err_msg);
}

BOOST_AUTO_TEST_CASE(unsupported_physics_parameters)
{
  // Incorrectly included parameter in "physics" block
  std::string const config{"process_name: pathMisspecification "
                           "physics: { "
                           "  producers : {} "
                           "  filters   : {} "
                           "  analyzers : {} "
                           "  test : atom "
                           "  check : { "
                           "    cannot : put "
                           "    random : table } }"};
  std::string const err_msg{
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
    "---- Configuration END\n"};
  check_exception(config, err_msg);
}
BOOST_AUTO_TEST_SUITE_END()
