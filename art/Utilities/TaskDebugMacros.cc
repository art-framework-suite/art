#include "art/Utilities/TaskDebugMacros.h"
#include "tbb/concurrent_unordered_map.h"
// vim: set sw=2 expandtab :

#include <cassert>
#include <cstdlib>

namespace {
  std::string
  banner(char const prefix)
  {
    std::ostringstream oss;
    oss << std::string(5, prefix) << "> " << std::setw(6) << std::right
        << std::this_thread::get_id() << std::left << " ";
    return oss.str();
  }

  std::string
  schedule_to_str(art::ScheduleID const id)
  {
    if (id == art::ScheduleID{}) {
      return std::string(4, ' ');
    }
    std::ostringstream oss;
    oss << "[" << std::setw(2) << std::right << std::setfill('0') << id
        << std::setfill(' ') << std::left << "]";
    return oss.str();
  }

  std::string
  trimmed(std::string const& fcn_name, std::string const& pretty_fcn_name)
  {
    // Use rfind because c'tor will have a pretty function name like:
    // art::Path::Path
    auto const begin_fcn_name = pretty_fcn_name.rfind(fcn_name);
    auto const begin_qualified_fcn_name =
      pretty_fcn_name.find_last_of(' ', begin_fcn_name);
    auto begin =
      begin_qualified_fcn_name > begin_fcn_name ? 0 : begin_qualified_fcn_name;
    // Remove art:: namespace qualification
    if (auto const stripped_begin = pretty_fcn_name.find("art::", begin);
        stripped_begin != std::string::npos) {
      begin = stripped_begin + 5;
    };
    auto const substr_length = begin_fcn_name - begin;
    return pretty_fcn_name.substr(begin, substr_length) + fcn_name;
  }

  tbb::concurrent_unordered_map<art::ScheduleID, unsigned> indents;

  std::string
  indent_for(std::string const& step, art::ScheduleID const sid)
  {
    auto it = indents.insert(std::make_pair(sid, 0)).first;
    if (step == "Begin") {
      auto const printed = it->second;
      ++it->second;
      return std::string(printed, ' ');
    } else if (step == "End") {
      auto const printed = --it->second;
      return std::string(printed, ' ');
    }
    return std::string(it->second, ' ');
  }
}

namespace art {

  DebugTasksValue::DebugTasksValue()
  {
    cvalue_ = getenv("ART_DEBUG_TASKS");
    value_ = (cvalue_.load() == nullptr) ? 0 : atoi(cvalue_.load());
  }

  DebugTasksValue debugTasks;

  namespace detail {
    MessageAccumulator::MessageAccumulator(char const banner_prefix,
                                           std::string const& fcn_name,
                                           std::string const& pretty_fcn_name,
                                           ScheduleID const sid,
                                           std::string const& step)
    {
      buffer_ << banner(banner_prefix) << schedule_to_str(sid) << " "
              << indent_for(step, sid) << std::left << std::setw(6) << step
              << trimmed(fcn_name, pretty_fcn_name);
    }

    MessageAccumulator::~MessageAccumulator()
    {
      auto const user_message = usr_msg_.str();
      if (not empty(user_message)) {
        buffer_ << " - " << user_message;
      }
      buffer_ << '\n';
      std::cerr << buffer_.str();
    }
  }

} // namespace art
