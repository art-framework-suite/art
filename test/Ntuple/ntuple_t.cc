#include <assert.h>
#include <cstring>
#include <iostream>

#include "art/Ntuple/Ntuple.h"

#include "sqlite3.h"

// We do not use Boost Unit Test here because we want the product into
// which this moves to be independent of Boost.

using namespace ntuple;

void nullptr_build_failure()
{
  std::cout << "start nullptr_build_failure\n";
  sqlite3* db = nullptr;
  try
    {
      Ntuple<int, double> t2(db, "table1", {"i", "x"});
      assert("Ntuple creation failed to throw required exception" == 0);
    }
  catch (std::runtime_error const& x) { }
  catch (...) { assert("Ntuple creation throw the wrong type of exception" == 0); }
  std::cout << "end nullptr_build_failure\n";
}

void test_with_new_database(sqlite3* db)
{
  std::cout << "start test_with_new_database\n";
  assert(db);
  Ntuple<double, std::string> xx(db, "xx", {"x", "txt"});
  std::cout << "end test_with_new_database\n";
}

void test_with_matching_table(sqlite3* db)
{
  std::cout << "start test_with_matching_table\n";
  assert(db);
  Ntuple<double, std::string> xx(db, "xx", {"x", "txt"});
  std::cout << "end test_with_matching_table\n";
}

template <class ... ARGS>
void test_with_colliding_table(sqlite3* db,
                               std::array<std::string, sizeof...(ARGS)> const& names)
{
  std::cout << "start test_with_colliding_table\n";
  assert(db);
  try
    {
      Ntuple<ARGS...> xx(db, "xx", names);
      assert("Failed throw for mismatched table" == 0);
    }
  catch (std::runtime_error const& x) { }
  catch (...) { assert("Threw wrong exception for mismatched table" == 0); }
  std::cout << "end test_with_colliding_table\n";
}

int count_rows(void* p, int nrows, char** results, char** cnames)
{
  int* n = static_cast<int*>(p);
  assert(nrows == 1);
  assert(std::strcmp(cnames[0], "count(*)") == 0);
  *n = std::atoi(results[0]);
  return 0;
}

void test_filling_table(sqlite3* db)
{
  std::cout << "start test_filling_table\n";
  assert(db);
  constexpr int nrows { 903 };
  {
    Ntuple<int, double> nt(db, "zz", {"i", "x"}, 100);
    for (int i = 0; i < nrows; ++i)
      {
        std::cout << "inserting row " << i << "\n";
        nt.insert(i, 1.5 * i);
      }
  }
  // Check that there are 'nrows' rows in the database.
  int nmatches = 0;
  char* errmsg;
  int rc = sqlite3_exec(db, "select count(*) from \"zz\"", count_rows, &nmatches, &errmsg);
  assert(rc == SQLITE_OK);
  assert(nmatches == nrows);
  std::cout << "end test_filling_table\n";
}

void test_file_create()
{
  const char* filename = "myfile.db";
  remove(filename);
  {
    Ntuple<int, double, int> table(filename, "tab1", {"i", "x", "k" }, 5);
    for (std::size_t i = 0; i < 103; ++i) table.insert(i, 0.5*i, i*i);
  }
  sqlite3* db;
  assert(sqlite3_open(filename, &db) == SQLITE_OK);
  assert(db);
  std::string query { "SELECT count(*) as cnt FROM tab1" };
  sqlite3_stmt* select_stmt;
  assert(sqlite3_prepare_v2(db, query.c_str(), query.size(), &select_stmt, nullptr) == SQLITE_OK);
  assert(sqlite3_step(select_stmt) == SQLITE_ROW);
  assert(sqlite3_column_int(select_stmt, 0) == 103);
  assert(sqlite3_finalize(select_stmt) == SQLITE_OK);
  sqlite3_close(db);
}

int main()
try
  {
    const char* fname { "no_such_file.db" };
    nullptr_build_failure();
    sqlite3* db = nullptr;
    // If there is a database in the directory, delete it.
    remove(fname);
    // Now make a database we are sure is empty.
    int status = sqlite3_open_v2(fname,
                                 &db,
                                 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                 nullptr);
    assert(status == 0);
    test_with_new_database(db);
    test_with_matching_table(db);
    test_with_colliding_table<int, double>(db, {"y", "txt"});
    test_with_colliding_table<int, double>(db, {"x", "text"});
    test_with_colliding_table<int, int>(db, {"x", "txt"});
    test_with_colliding_table<int, double, int>(db, {"x", "txt", "z"});
    test_with_colliding_table<int>(db, {"x"});
    test_filling_table(db);
    // Close database.
    sqlite3_close(db);

    test_file_create();
  }
catch (std::exception const& x)
  {
    std::cout << x.what() << std::endl;
    return 1;
  }
catch (...)
  {
    std::cout << "Unknown exception caught\n";
    return 2;
  }
