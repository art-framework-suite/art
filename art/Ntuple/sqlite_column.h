#ifndef art_Ntuple_sqlite_column_h
#define art_Ntuple_sqlite_column_h

// =======================================================
//
// sqlite helpers
//
// =======================================================

#include <array>
#include <string>
#include <utility>

namespace sqlite {

  template <size_t N> using name_array = std::array<std::string, N>;

  class column_base {
  public:
    column_base(std::string const& n) : name_{n} {}
    auto const& name() const { return name_; }
  private:
    std::string name_;
  };

  // column<T> is a containing struct that knows its c++ type (T) and
  // the sqlite translation (sqlite_type()).  There is no
  // implementation of the general case; the template must be
  // specialized for each supported type.
  template <typename T>
  struct column : column_base {
    using column_base::column_base;
  };

  template <>
  struct column<double> : column_base {
    using column_base::column_base;
    using type = double;
    std::string sqlite_type() const { return " numeric"; }
  };

  template <>
  struct column<float> : column_base {
    using column_base::column_base;
    using type = float;
    std::string sqlite_type() const { return " numeric"; }
  };

  template <>
  struct column<int> : column_base {
    using column_base::column_base;
    using type = int;
    std::string sqlite_type() const { return " integer"; }
  };

  template <>
  struct column<long> : column_base {
    using column_base::column_base;
    using type = long;
    std::string sqlite_type() const { return " integer"; }
  };

  template <>
  struct column<long long> : column_base {
    using column_base::column_base;
    using type = long long;
    std::string sqlite_type() const { return " integer"; }
  };

  template <>
  struct column<unsigned int> : column_base {
    using column_base::column_base;
    using type = int;
    std::string sqlite_type() const { return " integer"; }
  };

  template <>
  struct column<unsigned long> : column_base {
    using column_base::column_base;
    using type = long;
    std::string sqlite_type() const { return " integer"; }
  };

  template <>
  struct column<unsigned long long> : column_base {
    using column_base::column_base;
    using type = long long;
    std::string sqlite_type() const { return " integer"; }
  };

  template <>
  struct column<std::string> : column_base {
    using column_base::column_base;
    using type = std::string;
    std::string sqlite_type() const { return " text"; }
  };

  template <typename... ARGS>
  using column_pack = std::tuple<column<ARGS>...>;

  template <typename... ARGS, std::size_t... I>
  auto make_column_pack(name_array<sizeof...(ARGS)> const& cnames, std::index_sequence<I...>)
  {
    return column_pack<ARGS...>{cnames[I]...};
  }

} //namespace sqlite

#endif /* art_Ntuple_sqlite_helpers_h */

// Local Variables:
// mode: c++
// End:
