#ifndef sqlite_helpers_h
#define sqlite_helpers_h

#include "art/Ntuple/sqlite_result.h"
#include <string>

#include "sqlite3.h"

namespace sqlite {

  result query(sqlite3* db, std::string const& ddl);

  struct CompleteQuery {
    CompleteQuery(std::string&& ddl, sqlite3* const db) : ddl_{std::move(ddl)}, db_{db} {}
    std::string ddl_;
    sqlite3* db_;

    auto where(std::string const& cond) &&
    {
      ddl_ += " where ";
      ddl_ += cond;
      return CompleteQuery{std::move(ddl_), db_};
    }

    auto order_by(std::string const& column, std::string const& clause = {}) &&
    {
      ddl_ += " order by ";
      ddl_ += column;
      ddl_ += " "+clause;
      return CompleteQuery{std::move(ddl_), db_};
    }

    auto limit(int const num) &&
    {
      ddl_ += " limit ";
      ddl_ += std::to_string(num);
      return CompleteQuery{std::move(ddl_), db_};
    }

  };

  struct IncompleteQuery {

    IncompleteQuery(std::string&& ddl) : ddl_{std::move(ddl)} {}

    template <typename TUP>
    auto from(TUP const& t) &&
    {
      ddl_ += " from ";
      ddl_ += t.name();
      return CompleteQuery{std::move(ddl_), t.db()};
    }
    std::string ddl_;

  };

  namespace detail {
    inline std::string concatenate() { return ""; }

    template <typename H, typename... T>
    std::string concatenate(H const& h, T const&... t)
    {
      return sizeof...(t) != 0u ? std::string{h}+","+concatenate(t...) : std::string{h};
    }
  }

  template <typename... T>
  auto select(T const&... t)
  {
    std::string result {"select "+detail::concatenate(t...)};
    return IncompleteQuery{std::move(result)};
  }

  inline auto select_distinct(std::string const& column)
  {
    std::string result {"select distinct "+column};
    return IncompleteQuery{std::move(result)};
  }

  void operator<<(result& r, CompleteQuery const& cq);

}

#endif

// Local variables:
// mode: c++
// End:
