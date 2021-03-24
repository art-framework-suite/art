#ifndef art_Persistency_Provenance_PathSpec_h
#define art_Persistency_Provenance_PathSpec_h

#include <limits>
#include <string>
#include <tuple>
#include <utility>

namespace art {
  // Process name <-> Path name
  std::pair<std::string, std::string> split_process_and_path_names(
    std::string path_spec);

  class PathID {
  public:
    constexpr explicit PathID(size_t const i) : id_{i} {}

    constexpr auto static invalid() { return PathID{}; }

    constexpr bool
    operator==(PathID const other) const
    {
      return id_ == other.id_;
    }

    constexpr bool
    operator<(PathID const other) const
    {
      return id_ < other.id_;
    }

    friend std::string to_string(PathID);

  private:
    constexpr PathID() = default;
    size_t id_{std::numeric_limits<size_t>::max()};
  };

  struct PathSpec {
    std::string name;
    PathID path_id;
  };

  inline bool
  operator<(PathSpec const& a, PathSpec const b)
  {
    return std::tie(a.name, a.path_id) < std::tie(b.name, b.path_id);
  }

  // Conversion facilities for PathSpec <-> stringized form
  std::string to_string(PathID id);
  std::string to_string(PathSpec const& spec);
  PathSpec path_spec(std::string const& path_spec_str);
}

#endif /* art_Persistency_Provenance_PathSpec_h */

// Local Variables:
// mode: c++
// End:
