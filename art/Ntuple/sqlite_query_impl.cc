#include "art/Ntuple/sqlite_query_impl.h"

#include <cassert>

namespace {
  int getResult(void* data, int ncols, char** results, char** cnames)
  {
    assert(ncols >= 1);
    auto j = static_cast<sqlite::result*>(data);
    if (j->columns.empty()) {
      for (int i{}; i < ncols ; ++i)
        j->columns.push_back(cnames[i]);
    }

    sqlite::stringstream resultStream;
    for (int i{}; i < ncols ; ++i)
      resultStream << results[i];
    j->data.emplace_back(std::move(resultStream));
    return 0;
  }
}

namespace sqlite {

  result query(sqlite3* db, std::string const& ddl)
  {
    result res;
    char* errmsg {nullptr};
    if (sqlite3_exec(db, ddl.c_str(), getResult, &res, &errmsg) != SQLITE_OK) {
      std::string msg{errmsg};
      sqlite3_free(errmsg);
      throw art::Exception(art::errors::SQLExecutionError, msg);
    }
    return res;
  }

  void operator<<(result& r, CompleteQuery const& cq)
  {
    r = query(cq.db_, cq.ddl_+";");
  }

}
