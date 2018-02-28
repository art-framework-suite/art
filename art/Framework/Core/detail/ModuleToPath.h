#ifndef art_Framework_Core_detail_ModuleToPath_h
#define art_Framework_Core_detail_ModuleToPath_h

#include <map>
#include <string>
#include <vector>

namespace art {
  namespace detail {
    using path_name_t = std::string;
    using module_name_t = std::string;
    using names_t = std::vector<std::string>;
    using paths_to_modules_t = std::map<path_name_t, names_t>;
    using modules_to_paths_t = std::vector<std::pair<module_name_t, names_t>>;
    using distance_t = modules_to_paths_t::difference_type;

    class ModuleToPath {
    public:

      explicit ModuleToPath(paths_to_modules_t const& paths);
      auto const& name(std::size_t const i) const { return modules_[i].first; }
      auto const& paths(std::size_t const i) const { return modules_[i].second; }
      auto const& names() const { return modules_; }
      auto size() const { return modules_.size(); }
      auto find_vertex_index(std::string const& name) const -> distance_t;

    private:
      modules_to_paths_t const modules_;
      modules_to_paths_t::const_iterator const begin_;
      modules_to_paths_t::const_iterator const end_;
    };
  }
}

#endif
