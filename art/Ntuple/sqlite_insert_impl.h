#ifndef sqlite_insert_impl_h
#define sqlite_insert_impl_h

#include <initializer_list>
#include <string>
#include <vector>

#include "sqlite3.h"

namespace sqlite {

  template <typename TUP>
  struct IncompleteInsert {

    IncompleteInsert(TUP& t, std::initializer_list<std::string> columns)
      : table_{t}
      , columns_{columns}
    {}

    template <typename... T>
    void values(T const&... t) &&
    {
      table_.insert(t...);
    }

    TUP& table_;
    std::vector<std::string> columns_;
  };

  template <typename TUP, typename... C>
  auto insert_into(TUP& t, C const&... columns)
  {
    std::initializer_list<std::string> column_list {columns...};
    return IncompleteInsert<TUP>{t, column_list};
  }

}

#endif

// Local variables:
// mode: c++
// End:
