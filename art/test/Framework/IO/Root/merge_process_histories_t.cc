#define BOOST_TEST_MODULE (MergeProcessHistories_t)
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/IO/Root/mergeProcessHistories.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/test_macros.h"

using art::ProcessConfiguration;
using art::ProcessHistory;

namespace {
  template <typename... Args>
  ProcessHistory
  history(Args&&... args)
  {
    ProcessHistory result;
    result.reserve(sizeof...(args));
    std::vector<std::string> process_names{args...};
    for (auto const& name : process_names) {
      result.push_back(ProcessConfiguration{name, {}, {}});
    }
    return result;
  }

  auto
  merge(ProcessHistory const& first, ProcessHistory const& second)
  {
    return merge_process_histories(first, second);
  }

  auto
  merge(ProcessHistory const& first, std::vector<ProcessHistory> const& others)
  {
    return merge_process_histories(first, others);
  }
}

BOOST_AUTO_TEST_SUITE(merge_process_histories_t)

BOOST_AUTO_TEST_CASE(Empty)
{
  auto const test = merge(history("first"), history());
  auto const ref = history("first");
  CET_CHECK_EQUAL_COLLECTIONS(test, ref);
}

BOOST_AUTO_TEST_CASE(Different)
{
  auto const test =
    merge(history("first", "second"), history("really", "different"));
  auto const ref = history("really", "different", "first", "second");
  CET_CHECK_EQUAL_COLLECTIONS(test, ref);
}

BOOST_AUTO_TEST_CASE(OneAfter)
{
  auto const test = merge(history("first"), history("first", "second"));
  auto const ref = history("first", "second");
  CET_CHECK_EQUAL_COLLECTIONS(test, ref);
}

BOOST_AUTO_TEST_CASE(OneBefore)
{
  auto const test = merge(history("first"), history("zeroth", "first"));
  auto const ref = history("zeroth", "first");
  CET_CHECK_EQUAL_COLLECTIONS(test, ref);
}

BOOST_AUTO_TEST_CASE(Interspersed_1)
{
  auto const test =
    merge(history("first", "third"), history("zeroth", "first", "second"));
  auto const ref = history("zeroth", "first", "second", "third");
  CET_CHECK_EQUAL_COLLECTIONS(test, ref);
}

BOOST_AUTO_TEST_CASE(Interspersed_2)
{
  auto const test = merge(history("first", "second", "third"),
                          history("zeroth", "first", "third", "fourth"));
  auto const ref = history("zeroth", "first", "second", "third", "fourth");
  CET_CHECK_EQUAL_COLLECTIONS(test, ref);
}

BOOST_AUTO_TEST_CASE(Inconsistent_1)
{
  BOOST_REQUIRE_EXCEPTION(
    merge(history("first", "second"), history("second", "first")),
    art::Exception,
    [](art::Exception const& e) {
      return e.categoryCode() == art::errors::MismatchedInputFiles;
    });
}

BOOST_AUTO_TEST_CASE(Inconsistent_2)
{
  BOOST_REQUIRE_EXCEPTION(merge(history("first", "blah", "second"),
                                history("bleet", "second", "oink", "first")),
                          art::Exception,
                          [](art::Exception const& e) {
                            return e.categoryCode() ==
                                   art::errors::MismatchedInputFiles;
                          });
}

BOOST_AUTO_TEST_CASE(Separate_Merge_Orders)
{
  auto const first = history("common");
  auto const second = history("common", "tracker");
  auto const third = history("common", "calo");

  auto const test_a = merge(first, {second, third});
  auto const test_b = merge(first, {third, second});

  CET_CHECK_EQUAL_COLLECTIONS(test_a, test_b);
}

BOOST_AUTO_TEST_SUITE_END()
