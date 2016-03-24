// dump_file_contributors.cc

#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/detail/getFileContributors.h"
#include "art/Persistency/RootDB/SQLite3Wrapper.h"
#include "art/Persistency/RootDB/tkeyvfs.h"
#include "boost/program_options.hpp"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "cetlib/container_algorithms.h"

#include "TError.h"
#include "TFile.h"

extern "C" {
#include "sqlite3.h"
}

#include <algorithm>
#include <iostream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>

namespace bpo = boost::program_options;

using std::ostream;
using std::string;
using std::vector;
using stringvec = vector<string>;

int print_table_names(TFile & file, ostream & output, ostream & errors);
int print_table(TFile & file, string const& table, ostream & output, ostream & errors);
// int print_contributors(TFile & file, unsigned rangeID, ostream & output, ostream & errors);
int print_contributors(TFile & file, ostream & output, ostream & errors);
int print_contributors(TFile & file, art::RunNumber_t const, ostream & output, ostream & errors);
int print_contributors(TFile & file, art::RunNumber_t const, art::SubRunNumber_t, ostream & output, ostream & errors);

int save_to_db(TFile & file, string const& extFileName, ostream & output, ostream & errors);

namespace {

  void RootErrorHandler(int const level,
                        bool const die,
                        char const * location,
                        char const * message)
  {
    // Ignore dictionary errors.
    if (level == kWarning &&
        (!die) &&
        strcmp(location, "TClass::TClass") == 0 &&
        std::string(message).find("no dictionary") != std::string::npos) {
      return;
    }
    else {
      // Default behavior
      DefaultErrorHandler(level, die, location, message);
    }
  }

  auto getColumnNames(sqlite3* db, std::string const& table)
  {
    sqlite3_stmt* stmt {nullptr};
    std::string const ddl {"PRAGMA TABLE_INFO ('"+table+"');"};
    sqlite3_prepare_v2(db, ddl.c_str(), -1, &stmt, nullptr);
    stringvec result;
    while(sqlite3_step(stmt) == SQLITE_ROW) {
      result.emplace_back(reinterpret_cast<char const*>(sqlite3_column_text(stmt,1)));
    }
    return result;
  }

  stringvec getTableNames(sqlite3* const db)
  {
    sqlite3_stmt* stmt {nullptr};
    std::string const ddl {"SELECT name FROM sqlite_master WHERE type='table';"};
    sqlite3_prepare_v2(db, ddl.c_str(), -1, &stmt, nullptr);
    stringvec result;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      result.emplace_back(reinterpret_cast<char const*>(sqlite3_column_text(stmt,0)));
    }
    return result;
  }

  /*
  ** This function is used to load the contents of a database file on disk
  ** into the "main" database of open database connection pInMemory, or
  ** to save the current contents of the database opened by pInMemory into
  ** a database file on disk. pInMemory is probably an in-memory database,
  ** but this function will also work fine if it is not.
  **
  ** Parameter zFilename points to a nul-terminated string containing the
  ** name of the database file on disk to load from or save to. If parameter
  ** isSave is non-zero, then the contents of the file zFilename are
  ** overwritten with the contents of the database opened by pInMemory. If
  ** parameter isSave is zero, then the contents of the database opened by
  ** pInMemory are replaced by data loaded from the file zFilename.
  **
  ** If the operation is successful, SQLITE_OK is returned. Otherwise, if
  ** an error occurs, an SQLite error code is returned.
  */
  int saveDb(sqlite3 *pInMemory, const char *zFilename){
    int rc {0};                        /* Function return code */
    sqlite3 *pFile {nullptr};          /* Database connection opened on zFilename */
    sqlite3_backup *pBackup {nullptr}; /* Backup object used to copy data */

    /* Open the database file identified by zFilename. Exit early if this fails
    ** for any reason. */
    rc = sqlite3_open(zFilename, &pFile);
    if( rc==SQLITE_OK ){

      /* Set up the backup procedure to copy from the "main" database of
      ** connection pFile to the main database of connection pInMemory.
      ** If something goes wrong, pBackup will be set to NULL and an error
      ** code and  message left in connection pTo.
      **
      ** If the backup object is successfully created, call backup_step()
      ** to copy data from pFile to pInMemory. Then call backup_finish()
      ** to release resources associated with the pBackup object.  If an
      ** error occurred, then  an error code and message will be left in
      ** connection pTo. If no error occurred, then the error code belonging
      ** to pTo is set to SQLITE_OK.
      */
      pBackup = sqlite3_backup_init(pFile, "main", pInMemory, "main");
      if( pBackup ){
        (void)sqlite3_backup_step(pBackup, -1);
        (void)sqlite3_backup_finish(pBackup);
      }
      rc = sqlite3_errcode(pFile);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void)sqlite3_close(pFile);
    return rc;
  }
}

int main(int argc, char * argv[])
{
  // ------------------
  // use the boost command line option processing library to help out
  // with command line options
  std::ostringstream descstr;
  descstr << argv[0] << " <options> [<source-file>]+";

  string tableName {};
  art::RunNumber_t run {-1u};
  art::SubRunNumber_t subrun {-1u};
  unsigned rangeID {-1u};
  string extFileName {};

  bpo::options_description desc {descstr.str()};
  desc.add_options()
    ("help,h", "produce help message")
    ("table-names", "print list of SQLite tables in Root file")
    ("table",bpo::value<string>(&tableName), "print table corresponding to provided argument")
    ("run",bpo::value<unsigned>(&run), "print contributors for specified run")
    ("subrun",bpo::value<unsigned>(&subrun), "print contributors for specified subrun")
    ("range-id",bpo::value<unsigned>(&rangeID), "print contributors corresponding to specified range ID")
    ("save-to-db",bpo::value<string>(&extFileName), "save RootFileDB to an external DB file")
    ("source,s",  bpo::value<stringvec>(), "source data file (multiple OK)");

  bpo::options_description all_opts {"All Options"};
  all_opts.add(desc);

  // Each non-option argument is interpreted as the name of a files to
  // be processed. Any number of filenames is allowed.
  bpo::positional_options_description pd;
  pd.add("source", -1);
  // The variables_map contains the actual program options.
  bpo::variables_map vm;
  try {
    bpo::store(bpo::command_line_parser(argc, argv).options(all_opts).positional(pd).run(), vm);
    bpo::notify(vm);
  }
  catch (bpo::error const & e) {
    std::cerr << "Exception from command line processing in "
              << argv[0] << ": " << e.what() << "\n";
    return 2;
  }

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  // Get the names of the files we will process.
  stringvec file_names;
  size_t const file_count = vm.count("source");
  if (file_count < 1) {
    std::cerr << "One or more input files must be specified;"
              << " supply filenames as program arguments\n"
              << "For usage and options list, please do '"<< argv[0] << " --help'.\n";
    return 3;
  }
  file_names.reserve(file_count);
  cet::copy_all(vm["source"].as<stringvec>(), std::back_inserter(file_names));

  SetErrorHandler(RootErrorHandler);
  tkeyvfs_init();

  bool const printTableNames = vm.count("table-names");

  ostream& output = std::cout;
  ostream& errors = std::cerr;

  int rc {0};
  for (auto const& fn : file_names) {
    std::unique_ptr<TFile> current_file(TFile::Open(fn.c_str(), "READ"));
    if (!current_file || current_file->IsZombie()) {
      ++rc;
      errors << "Unable to open file '"
             << fn
             << "' for reading."
             << "\nSkipping file.\n";
      continue;
    }

    auto* key_ptr = current_file->GetKey("RootFileDB");
    if (key_ptr == nullptr) {
      ++rc;
      errors  << "\nRequested DB, \"RootFileDB\" of type, \"tkeyvfs\", not present in file: \""
              << fn
              << "\"\n"
              << "Either this is not an art/ROOT file, it is a corrupt art/ROOT file,\n"
              << "or it is an art/ROOT file produced with a version older than v1_00_12.\n";
      continue;
    }

    if (printTableNames) {
      rc += print_table_names(*current_file, output, errors);
    }
    else if (!extFileName.empty()) {
      rc += save_to_db(*current_file, extFileName, output, errors);
      output << "\nRootFileDB from file \"" << current_file->GetName() << "\"\n"
             << "saved to external database \"" << extFileName << "\".\n";
    }
    else if (run != -1u && subrun == -1u) {
      rc += print_contributors(*current_file, run, output, errors);
    }
    else if (run != -1u && subrun != -1u) {
      rc += print_contributors(*current_file, run, subrun, output, errors);
    }
    // else if (rangeID != -1u) {
    //   rc += print_contributors(*current_file, rangeID, output, errors);
    // }
    else if (!tableName.empty()) {
      rc += print_table(*current_file, tableName, output, errors);
    }
  }
  output << '\n';
  return rc;
}

//============================================================================

int print_table_names(TFile & file,
                      ostream & output,
                      ostream & errors [[gnu::unused]])
{
  art::SQLite3Wrapper db {&file, "RootFileDB"};
  auto tableNames = getTableNames(db);
  output << "Tables\n"
         << std::string(30,'=') << '\n';
  for (auto const& name : tableNames )
    output << name << '\n';
  return 0;
}


int save_to_db(TFile & file,
               string const& extFileName,
               ostream & output [[gnu::unused]],
               ostream & errors [[gnu::unused]])
{
  art::SQLite3Wrapper db {&file, "RootFileDB"};
  return saveDb(db, extFileName.c_str());
}


int print_table(TFile & file,
                std::string const& table,
                ostream & output,
                ostream & errors)
{
  art::SQLite3Wrapper db {&file, "RootFileDB"};

  sqlite3_stmt* stmt {nullptr};
  bool row_found {false};
  auto columns = getColumnNames(db, table);

  std::string const ddl {"select * from " + table +";"};
  auto rc = sqlite3_prepare_v2(db, ddl.c_str(), -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    errors << "Error in preparing statement for FileContributors.\n";
    return 1;
  }

  output << "\nTable: " << table << '\n'
         << std::string(30,'=') << '\n';
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    row_found = true;
    for (std::size_t i{0}, n{columns.size()} ; i!=n ; ++i)
      output << columns[i] << " " << sqlite3_column_text(stmt,i) << ' ';
    output << '\n';
  }
  if (rc != SQLITE_DONE) {
    errors << "Unexpected status from table read: "
           << sqlite3_errmsg(db)
           << " (0x"
           << rc
           <<").\n";
  }

  auto const finalize_status = sqlite3_finalize(stmt);
  if (finalize_status != SQLITE_OK) {
    errors << "Unexpected status from DB status cleanup: "
           << sqlite3_errmsg(db)
           << " (0x"
           << finalize_status
           <<").\n";
  }
  if (!row_found) {
    errors << "FileContributors table is missing or empty.\n";
    return 1;
  }
  return 0;
}

int print_contributors(TFile & file,
                       art::RunNumber_t const r,
                       ostream & output,
                       ostream & errors)
{
  auto const& contributors = art::detail::getRunContributors(file, r);
  if ( contributors.empty() ) {
    errors << "Run " << r << " not found in file:\n"
           << file.GetName() << ".\n";
    return 1;
  }

  output << "\nFile contributors for Run: " << r << '\n'
         << std::string(30,'=') << '\n';
  for (auto const& entry : contributors)
    output << entry << '\n';
  return 0;
}

int print_contributors(TFile & file,
                       art::RunNumber_t const r,
                       art::SubRunNumber_t const sr,
                       ostream & output,
                       ostream & errors)
{
  auto const& contributors = art::detail::getSubRunContributors(file, r, sr);

  if ( contributors.empty() ) {
    errors << "Run " << r << " not found in file:\n"
           << file.GetName() << ".\n";
    return 1;
  }

  output << "\nFile contributors for Run: " << r << " SubRun: " << sr << '\n'
         << std::string(30,'=') << '\n';
  for (auto const& entry : contributors)
    output << entry << '\n';
  return 0;
}

// int print_contributors(TFile & file,
//                        unsigned const rangeSetID,
//                        ostream & output,
//                        ostream & errors)
// {
//   auto const& contributors = art::detail::getSubRunContributors(file,
//                                                                 rangeSetID);

//   if ( contributors.empty() ) {
//     errors << "RangeSetID " << rangeSetID << " not found in file:\n"
//            << file.GetName() << ".\n";
//     return 1;
//   }

//   output << "\nFile contributors for RangeSetID: " << rangeSetID << '\n'
//          << "Run: " << r
//          << std::string(30,'=') << '\n';
//   for (auto const& entry : contributors)
//     output << entry << '\n';
//   return 0;
// }
