#define BOOST_TEST_MODULE (path_specs test)
#include "boost/test/unit_test.hpp"

#include "art/Framework/Art/detail/prune_configuration.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

using namespace art;

namespace {
  std::string const trigger_paths{"trigger_paths"};

  auto
  path_specs_from(std::string const& spec_str)
  {
    auto const table = fhicl::parse_document(spec_str);
    auto const& sequence = table.find(trigger_paths);
    auto const path_entries = detail::sequence_to_entries(sequence, true);
    return detail::path_specs(path_entries, trigger_paths);
  }

  auto
  trigger_path_specs_for(std::string const& sequence_str)
  {
    return path_specs_from(trigger_paths + ": " + sequence_str);
  }

  auto
  spec_for(std::string const& name, size_t const i)
  {
    return PathSpec{name, PathID{i}};
  }
}

BOOST_AUTO_TEST_SUITE(path_specs_t)

BOOST_AUTO_TEST_CASE(empty_path_spec)
{
  BOOST_TEST(trigger_path_specs_for("[]").empty());
}

BOOST_AUTO_TEST_CASE(one_entry)
{
  auto const path_specs = trigger_path_specs_for("[a]");
  BOOST_TEST(size(path_specs) == 1ull);
  BOOST_TEST(path_specs[0] == spec_for("a", 0));
}

BOOST_AUTO_TEST_CASE(one_entry_with_bit_number)
{
  auto const path_specs = trigger_path_specs_for("['1:a']");
  BOOST_TEST(size(path_specs) == 1ull);
  BOOST_TEST(path_specs[0] == spec_for("a", 1));
}

BOOST_AUTO_TEST_CASE(entries_skipping_bits)
{
  auto const path_specs = trigger_path_specs_for("[a, '2:c', '3:d']");
  BOOST_TEST(size(path_specs) == 3ull);
  BOOST_TEST(path_specs[0] == spec_for("a", 0));
  BOOST_TEST(path_specs[1] == spec_for("c", 2));
  BOOST_TEST(path_specs[2] == spec_for("d", 3));
}

BOOST_AUTO_TEST_CASE(entries_skipping_bits_with_nil)
{
  auto const path_specs = path_specs_from("trigger_paths: [a]\n"
                                          "trigger_paths[100]: c");
  BOOST_TEST(size(path_specs) == 2ull);
  BOOST_TEST(path_specs[0] == spec_for("a", 0));
  BOOST_TEST(path_specs[1] == spec_for("c", 100));
}

BOOST_AUTO_TEST_CASE(repeated_path_name)
{
  BOOST_CHECK_EXCEPTION(
    trigger_path_specs_for("[a, a]"), art::Exception, [](auto const& e) {
      return e.categoryCode() == art::errors::Configuration and
             e.explain_self().find("has already been specified in the ") !=
               std::string::npos;
    });
  BOOST_CHECK_EXCEPTION(
    trigger_path_specs_for("[a, '1:a']"), art::Exception, [](auto const& e) {
      return e.categoryCode() == art::errors::Configuration and
             e.explain_self().find(
               "has already been specified (perhaps implicitly) ") !=
               std::string::npos;
    });
  BOOST_CHECK_EXCEPTION(
    trigger_path_specs_for("['0:a', '1:a']"),
    art::Exception,
    [](auto const& e) {
      return e.categoryCode() == art::errors::Configuration and
             e.explain_self().find(
               "has already been specified (perhaps implicitly) ") !=
               std::string::npos;
    });
  auto const path_specs = trigger_path_specs_for("['1:a', '1:a']");
  BOOST_CHECK(size(path_specs) == 1ull);
  BOOST_CHECK(path_specs[0] == spec_for("a", 1));
}

BOOST_AUTO_TEST_CASE(repeated_path_id)
{
  BOOST_CHECK_EXCEPTION(
    trigger_path_specs_for("['1:a', 'b']"), art::Exception, [](auto const& e) {
      return e.categoryCode() == art::errors::Configuration and
             e.explain_self().find("has already been assigned to path name ") !=
               std::string::npos;
    });
  BOOST_CHECK_EXCEPTION(
    trigger_path_specs_for("['3:a', '3:b']"),
    art::Exception,
    [](auto const& e) {
      return e.categoryCode() == art::errors::Configuration and
             e.explain_self().find("has already been assigned to path name ") !=
               std::string::npos;
    });
}

BOOST_AUTO_TEST_SUITE_END()
