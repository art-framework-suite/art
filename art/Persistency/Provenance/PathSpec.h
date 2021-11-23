#ifndef art_Persistency_Provenance_PathSpec_h
#define art_Persistency_Provenance_PathSpec_h

#include <iosfwd>
#include <limits>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace art {
  // Process name <-> Path name
  std::pair<std::string, std::string> split_process_and_path_names(
    std::string path_spec);

  class PathID {
  public:
    constexpr explicit PathID(size_t const i) noexcept : id_{i} {}

    constexpr auto static invalid() noexcept { return PathID{}; }

    constexpr bool
    operator==(PathID const other) const noexcept
    {
      return id_ == other.id_;
    }

    constexpr bool
    operator!=(PathID const other) const noexcept
    {
      return not operator==(other);
    }

    constexpr bool
    operator<(PathID const other) const noexcept
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

  inline auto
  to_tuple(PathSpec const& spec) noexcept
  {
    return std::tie(spec.name, spec.path_id);
  }

  inline bool
  operator==(PathSpec const& a, PathSpec const& b)
  {
    return to_tuple(a) == to_tuple(b);
  }

  inline bool
  operator<(PathSpec const& a, PathSpec const& b)
  {
    return to_tuple(a) < to_tuple(b);
  }

  // Conversion facilities for PathSpec <-> stringized form
  std::string to_string(PathID id);
  std::string to_string(PathSpec const& spec);
  PathSpec path_spec(std::string const& path_spec_str);
  std::vector<PathSpec> path_specs(
    std::vector<std::string> const& path_spec_strs);
  std::ostream& operator<<(std::ostream& os, PathSpec const& spec);
}

#endif /* art_Persistency_Provenance_PathSpec_h */

// Local Variables:
// mode: c++
// End:
