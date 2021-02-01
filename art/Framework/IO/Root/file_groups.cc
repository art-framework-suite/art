#include "art/Framework/IO/Root/file_groups.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/split.h"

#include "boost/algorithm/string.hpp"
#include "boost/range/algorithm_ext.hpp"

#include <iostream>

namespace {
  void
  remove_commented_characters(std::string& line)
  {
    auto begin = line.find('#');
    auto next_end_of_line = line.find_first_of("\r\n");
    while (begin != std::string::npos and
           next_end_of_line != std::string::npos) {
      auto const diff = next_end_of_line - begin;
      line.erase(begin, diff);
      begin = line.find('#');
      next_end_of_line = line.find_first_of("\r\n", begin);
    }

    // Remove comment from final line, which may not end with a '\r' or '\n'
    if (begin != std::string::npos) {
      line.erase(begin);
    }
  }

  void
  remove_whitespace(std::string& line)
  {
    boost::remove_erase_if(line, boost::is_any_of(" \t\r\n"));
  }

  auto
  bar_position(std::string const& line)
  {
    if (std::count(line.begin(), line.end(), '|') > 1u) {
      throw art::Exception{
        art::errors::Configuration,
        "There was an error in reading a line of the EventMerger file list.\n"}
        << "Only one vertical bar ('|') allowed per line:\n  " << line;
    }
    return line.find("|");
  }

  auto
  primary_file(std::string const& line,
               std::string::size_type const end_pos = std::string::npos)
  {
    auto const result = line.substr(0, end_pos);
    if (result.empty()) {
      throw art::Exception{
        art::errors::Configuration,
        "There was an error in reading a line of the EventMerger file list.\n"}
        << "The following line does not specify a primary file:\n  " << line;
    }

    if (std::count(result.begin(), result.end(), ',') > 0) {
      throw art::Exception{
        art::errors::Configuration,
        "There was an error in reading a line of the EventMerger file list.\n"}
        << "No commas are allowed in the specification of the primary file:\n  "
        << line;
    }

    return result;
  }
}

namespace art {
  bool
  file_group(std::string line, entry_t& group)
  {
    assert(std::count(line.begin(), line.end(), ';') == 0);

    remove_commented_characters(line);
    remove_whitespace(line);
    if (line.empty()) {
      return false;
    }

    auto const bar = bar_position(line);
    if (bar != std::string::npos) {
      auto primary = primary_file(line, bar);
      auto secondaries_str = line.substr(bar + 1);
      std::vector<std::string> secondaries;
      cet::split(secondaries_str, ',', back_inserter(secondaries));
      group = {move(primary), move(secondaries)};
      return true;
    }

    auto primary = primary_file(line);
    group = {move(primary), {}};
    return true;
  }

  collection_t
  file_groups(std::istream& file_list)
  {
    collection_t result;
    assert(file_list);
    for (std::string line; std::getline(file_list, line, ';');) {
      entry_t group;
      if (file_group(line, group)) {
        result.push_back(group);
      }
    }
    return result;
  }

}
