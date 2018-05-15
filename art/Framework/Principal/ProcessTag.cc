#include "art/Framework/Principal/ProcessTag.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

using namespace std::string_literals;

namespace {
  std::string const current_process_lit{"current_process"};
  std::string const input_source_lit{"input_source"};

  auto
  allowed_search_policy(std::string const& specified_name)
  {
    using allowed_search = art::ProcessTag::allowed_search;
    if (specified_name == current_process_lit) {
      return allowed_search::current_process;
    } else if (specified_name == input_source_lit) {
      return allowed_search::input_source;
    }
    return allowed_search::all_processes;
  }

  std::string
  process_name(std::string const& specified_name,
               std::string const& current_process_name)
  {
    return specified_name == current_process_lit ? current_process_name :
                                                   specified_name;
  }
}

art::ProcessTag::ProcessTag() = default;

art::ProcessTag::ProcessTag(std::string const& specified_name)
  : name_{specified_name}
{}

art::ProcessTag::ProcessTag(std::string const& specified_name,
                            std::string const& current_process_name)
  : search_{allowed_search_policy(specified_name)}
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
