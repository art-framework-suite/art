#include "art/Ntuple/sqlite_result.h"
#include <ostream>

namespace sqlite {

  std::ostream& operator<<(std::ostream& os, result const& res)
  {
    using size_t = decltype(res.columns.size());
    auto const ncolumns = res.columns.size();
    for (size_t i{}; i!= ncolumns ; ++i) {
      os << res.columns[i] << ' ';
    }
    os << "\n--------------------------------\n";
    for (auto const& row : res.data) {
      for (size_t i{}; i!= ncolumns ; ++i) {
        os << row[i] << ' ';
      }
      os << '\n';
    }
    return os;
  }

} //namespace sqlite
