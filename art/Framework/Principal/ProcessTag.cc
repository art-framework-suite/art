#include "art/Framework/Principal/ProcessTag.h"

using namespace std::string_literals;

namespace {
  std::string const current_process_lit{"current_process"};
  std::string const input_source_lit{"input_source"};

  auto
  allowed_search_policy(std::string const& specified_name,
                        std::string const& current_process_name)
  {
    using allowed_search = art::ProcessTag::allowed_search;
    // If user specifies the literal "current_process" or a name that
    // matches the current process name, then only current-process
    // searching is allowed.
    if (specified_name == current_process_lit ||
        specified_name == current_process_name) {
      return allowed_search::current_process;
    }

    // If user specifies the literal "input_source" or a non-empty
    // process name (that does not correspond to the current process,
    // which is handled above), then only input-source searching is
    // allowed.
    if (specified_name == input_source_lit || !specified_name.empty()) {
      return allowed_search::input_source;
    }

    // Default to searching all processes
    return allowed_search::all_processes;
  }

  std::string
  process_name(std::string const& specified_name,
               std::string const& current_process_name)
  {
    if (specified_name == current_process_lit) {
      return current_process_name;
    } else if (specified_name == input_source_lit) {
      return {};
    }
    return specified_name;
  }
}

art::ProcessTag::ProcessTag() = default;

art::ProcessTag::ProcessTag(std::string const& specified_name)
  : name_{specified_name}
{}

art::ProcessTag::ProcessTag(std::string const& specified_name,
                            std::string const& current_process_name)
  : search_{allowed_search_policy(specified_name, current_process_name)}
  , name_{process_name(specified_name, current_process_name)}
{}

bool
art::ProcessTag::input_source_search_allowed() const
{
  using allowed_search = ProcessTag::allowed_search;
  return search_ == allowed_search::all_processes ||
         search_ == allowed_search::input_source;
}

bool
art::ProcessTag::current_process_search_allowed() const
{
  using allowed_search = ProcessTag::allowed_search;
  return search_ == allowed_search::all_processes ||
         search_ == allowed_search::current_process;
}
