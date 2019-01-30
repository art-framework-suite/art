#include "art/Framework/Art/detail/event_start.h"

#include "boost/algorithm/string.hpp"
#include "canvas/Utilities/Exception.h"

#include <iostream>
#include <regex>
#include <sstream>
#include <type_traits>

namespace {
  // Only two forms of EventID specification are allowed:
  //   - One non-zero number (deprecated)
  //   - A triplet of non-zero numbers, separated by the colon (e.g. "1:2:3")
  // Numbers are allowed to have leading and trailing whitespace.
  auto
  group(std::string const& spec)
  {
    return "(" + spec + ")";
  }

  auto
  maybe(std::string const& spec)
  {
    return group(spec) + "?";
  }

  std::string const number{R"(\s*\d+\s*)"};
  std::regex const re_event_id{
    maybe(group(number) + ":" + group(number) + ":") + group(number)};
  std::string const context{"An error was encountered while processing the "
                            "-e|--estart program option.\n"};

  constexpr auto article [[maybe_unused]] (art::Level const L)
  {
    return (L == art::Level::InputFile || L == art::Level::Event) ? "an" : "a";
  }

  template <art::Level L>
  auto
  safe_conversion(std::string str_num)
  {
    boost::trim(str_num);
    auto const num = std::stoull(str_num);

    if (num > art::IDNumber<L>::max_valid() ||
        num < art::IDNumber<L>::first()) {
      // No need to provide exception message since it will be
      // included in the rethrown exception.
      throw art::Exception{art::errors::Configuration};
    }
    return static_cast<art::IDNumber_t<L>>(num);
  }

  template <art::Level L>
  auto
  range()
  {
    std::stringstream oss;
    oss << '[' << art::IDNumber<L>::first() << ", "
        << art::IDNumber<L>::invalid() << ')';
    return oss.str();
  }

  [[noreturn]] void
  throw_configuration_exception(std::string const& spec) noexcept(false)
  {
    throw art::Exception{art::errors::Configuration, context}
      << "The specification '" << spec << "' is not a valid EventID.\n"
      << "Please specify a value of the form '<run>:<subrun>:<event>' where:\n"
      << "  <run>    is in the range " << range<art::Level::Run>() << '\n'
      << "  <subrun> is in the range " << range<art::Level::SubRun>() << '\n'
      << "  <event>  is in the range " << range<art::Level::Event>() << '\n';
  }

  template <art::Level L>
  auto
  convert_or_throw(std::string const& field,
                   std::string const& event_spec) noexcept(false) try {
    return safe_conversion<L>(field);
  }
  catch (...) {
    throw_configuration_exception(event_spec);
  }
}

std::tuple<art::RunNumber_t, art::SubRunNumber_t, art::EventNumber_t>
art::detail::event_start(std::string const& event_spec)
{
  std::smatch parts;
  auto const success = std::regex_match(event_spec, parts, re_event_id);
  if (!success) {
    throw_configuration_exception(event_spec);
  }

  assert(parts.size() == 5ull);
  // A successful match will populate 'parts' with 5 elements.
  // Consider the following valid specifications:
  //
  //   +-------------+---------+-----+
  //   | User spec.  | '1:0:3' | '3' |
  //   +-------------+---------+-----+
  //   | parts[0] == | '1:0:3' | '3' |
  //   | parts[1] == |  '1:0:' |  '' |
  //   | parts[2] == |     '1' |  '' |
  //   | parts[3] == |     '0' |  '' |
  //   | parts[4] == |     '3' | '3' |
  //   +-------------+---------+-----+

  auto run = IDNumber<Level::Run>::first();
  auto subrun = IDNumber<Level::SubRun>::first();
  auto const event = convert_or_throw<Level::Event>(parts[4], event_spec);

  // Test the deprecated case.
  if (parts[1].str().empty()) {
    assert(parts[2].str().empty() && parts[3].str().empty());
    std::cerr
      << "\nSpecifying an event number of " << event
      << " is now deprecated when using\n"
      << "the -e|--estart program option.  Please explicitly specify the\n"
      << "run and subrun numbers (e.g.):\n"
      << "   -e \"1:0:" << event << "\"\n\n";
  } else {
    run = convert_or_throw<Level::Run>(parts[2], event_spec);
    subrun = convert_or_throw<Level::SubRun>(parts[3], event_spec);
  }

  return std::make_tuple(run, subrun, event);
}
