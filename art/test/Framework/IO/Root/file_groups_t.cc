#define BOOST_TEST_MODULE (form_file_names_t)
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/IO/Root/file_groups.h"
#include "canvas/Utilities/Exception.h"

#include <sstream>
#include <string>

namespace {
  auto
  to_entry(std::string const& primary,
           std::vector<std::string> const& secondaries = {})
  {
    return art::entry_t{primary, secondaries};
  }
}

using art::file_group;
using art::file_groups;

BOOST_AUTO_TEST_SUITE(file_groups_t)

BOOST_AUTO_TEST_CASE(single_line_t)
{
  art::entry_t group;

  // No groups
  BOOST_TEST(not file_group("", group));
  BOOST_TEST(not file_group("# Comment only", group));
  BOOST_TEST(not file_group("  # Comment with leading white space", group));

  // Single primary file
  BOOST_TEST(file_group("primary", group));
  BOOST_TEST(std::get<0>(group) == "primary");
  BOOST_TEST(std::get<1>(group).empty());

  // Single primary file with vertical bar
  BOOST_TEST(file_group("primary|", group));
  BOOST_TEST(std::get<0>(group) == "primary");
  BOOST_TEST(std::get<1>(group).empty());

  // Primary with one secondary
  BOOST_TEST(file_group("primary|secondary", group));
  BOOST_TEST(std::get<0>(group) == "primary");
  BOOST_TEST(std::get<1>(group) == std::vector<std::string>{"secondary"});

  // Only secondary file
  BOOST_CHECK_EXCEPTION(
    file_group("|secondary", group), art::Exception, [](auto const& e) {
      return e.explain_self().find(
               "The following line does not specify a primary file") !=
             std::string::npos;
    });

  // Secondary files with comments
  BOOST_TEST(file_group("primary # comment to end of line\n\r"
                        "|secondary1, # One more comment\n"
                        "secondary2, secondary3",
                        group));
  BOOST_TEST(std::get<0>(group) == "primary");
  BOOST_TEST(
    std::get<1>(group) ==
    (std::vector<std::string>{"secondary1", "secondary2", "secondary3"}));
}

BOOST_AUTO_TEST_CASE(empty_file)
{
  std::istringstream empty;
  auto const test = file_groups(empty);
  auto const ref = art::collection_t{};
  BOOST_TEST(test == ref);
}

BOOST_AUTO_TEST_CASE(empty_lines)
{
  std::istringstream lines_with_whitespace{" ;;\r\n\t  ;  "};
  auto const test = file_groups(lines_with_whitespace);
  auto const ref = art::collection_t{};
  BOOST_TEST(test == ref);
}

BOOST_AUTO_TEST_CASE(single_file)
{
  std::istringstream primary_file{"primary"};
  auto const test = file_groups(primary_file);
  auto const ref = art::collection_t{to_entry("primary")};
  BOOST_TEST(test == ref);
}

BOOST_AUTO_TEST_CASE(multiple_files)
{
  std::istringstream multiple_files{"primary1;primary2;primary3"};
  auto const test = file_groups(multiple_files);
  auto const ref = art::collection_t{
    to_entry("primary1"), to_entry("primary2"), to_entry("primary3")};
  BOOST_TEST(test == ref);
}

BOOST_AUTO_TEST_CASE(more_complicated_structure)
{
  std::istringstream multiple_files{"primaryA|secondaryA1,secondaryA2;\n"
                                    "primaryB|secondaryB1; #secondaryB2\n"
                                    "primaryC|\n"
                                    "  secondaryC1,\n"
                                    "  # secondaryC2 <- Buggy file\n"
                                    "  secondaryC3"};
  auto const test = file_groups(multiple_files);
  auto const ref =
    art::collection_t{to_entry("primaryA", {"secondaryA1", "secondaryA2"}),
                      to_entry("primaryB", {"secondaryB1"}),
                      to_entry("primaryC", {"secondaryC1", "secondaryC3"})};
  BOOST_TEST(test == ref);
}

BOOST_AUTO_TEST_SUITE_END()
