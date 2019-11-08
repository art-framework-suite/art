// vim: set sw=2 expandtab :
#define BOOST_TEST_MODULE (prune_config test)
#include "cetlib/quiet_unit_test.hpp"

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

BOOST_AUTO_TEST_SUITE(PathManager_t)

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
    "  An error was encountered while processing module configurations.\n"
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
    "  An error was encountered while processing a path configuration.\n"
    "  The following entries in path p1 are observers when all other\n"
    "  entries are modifiers:\n"
    "    'a'\n"
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
    "  The following error was encountered while processing a path "
    "configuration:\n"
    "  Entry f in path p1 does not have a module configuration.\n"
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
    "  The following error was encountered while processing a path "
    "configuration:\n"
    "  Entry f in path p1 does not have a module configuration.\n"
    "---- Configuration END\n"};
  check_exception(config, err_msg);
}

BOOST_AUTO_TEST_SUITE_END()
