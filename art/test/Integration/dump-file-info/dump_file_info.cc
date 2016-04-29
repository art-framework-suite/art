// dump_file_info.cc

#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/detail/resolveRangeSet.h"
#include "art/Persistency/RootDB/SQLite3Wrapper.h"
#include "art/Persistency/RootDB/tkeyvfs.h"
#include "boost/program_options.hpp"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "cetlib/container_algorithms.h"
#include "art/test/Integration/dump-file-info/InputFile.h"

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

using namespace std::string_literals;
using art::detail::InputFile;
using std::ostream;
using std::string;
using std::vector;
using stringvec = vector<string>;


int print_range_sets(InputFile& file, ostream& output);
int print_file_index(InputFile& file, ostream& output);
int db_to_file(InputFile& file, ostream& output, ostream& errors);

namespace {

  void RootErrorHandler(int const level,
                        bool const die,
                        char const* location,
                        char const* message)
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

  // Code taken from the SQLite webpage:
  //     https://www.sqlite.org/backup.html
  // With modifications.
  //
  // SQLite comments:
  //
  // This function is used to load the contents of a database file on disk
  // into the "main" database of open database connection pInMemory, or
  // to save the current contents of the database opened by pInMemory into
  // a database file on disk. pInMemory is probably an in-memory database,
  // but this function will also work fine if it is not.
  //
  // Parameter zFilename points to a nul-terminated string containing the
  // name of the database file on disk to load from or save to. If parameter
  // isSave is non-zero, then the contents of the file zFilename are
  // overwritten with the contents of the database opened by pInMemory. If
  // parameter isSave is zero, then the contents of the database opened by
  // pInMemory are replaced by data loaded from the file zFilename.
  //
  // If the operation is successful, SQLITE_OK is returned. Otherwise, if
  // an error occurs, an SQLite error code is returned.

  int dbToFile(sqlite3 *pInMemory, const char *zFilename)
  {
    int rc {0};                        // Function return code
    sqlite3 *pFile {nullptr};          // Database connection opened on zFilename
    sqlite3_backup *pBackup {nullptr}; // Backup object used to copy data

    // Open the database file identified by zFilename. Exit early if this fails
    // for any reason.
    rc = sqlite3_open(zFilename, &pFile);
    if ( rc==SQLITE_OK ){

      // Set up the backup procedure to copy from the "main" database of
      // connection pFile to the main database of connection pInMemory.
      // If something goes wrong, pBackup will be set to NULL and an error
      // code and  message left in connection pTo.

      // If the backup object is successfully created, call backup_step()
      // to copy data from pFile to pInMemory. Then call backup_finish()
      // to release resources associated with the pBackup object.  If an
      // error occurred, then  an error code and message will be left in
      // connection pTo. If no error occurred, then the error code belonging
      // to pTo is set to SQLITE_OK.

      pBackup = sqlite3_backup_init(pFile, "main", pInMemory, "main");
      if(pBackup != nullptr){
        (void)sqlite3_backup_step(pBackup, -1);
        (void)sqlite3_backup_finish(pBackup);
      }
      rc = sqlite3_errcode(pFile);
    }

    // Close the database connection opened on database file zFilename
    // and return the result of this function.
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

  bpo::options_description desc {descstr.str()};
  desc.add_options()
    ("help,h", "produce help message")
    ("print-file-index", "prints FileIndex object for each input file")
    ("print-range-sets", "prints event range sets for each input file")
    ("db-to-file",
     ("Writes RootFileDB to external SQLite database with the same base name as the input file and the suffix '.db'.\n"s +
      "(Writes to directory in which '"s + argv[0] + "' is executed)."s).c_str())
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

  bool const printRangeSets = vm.count("print-range-sets") > 0;
  bool const printFileIndex = vm.count("print-file-index") > 0;
  bool const saveDbToFile   = vm.count("db-to-file") > 0;

  SetErrorHandler(RootErrorHandler);
  tkeyvfs_init();

  ostream& output = std::cout;
  ostream& errors = std::cerr;

  int rc {0};
  for (auto const& fn : file_names) {
    output << std::string(30,'=') << '\n'
           << "File: " << fn.substr(fn.find_last_of('/')+1ul) << '\n';
    InputFile file {fn};
    if (printRangeSets) rc += print_range_sets(file, output);
    if (printFileIndex) rc += print_file_index(file, output);
    if (saveDbToFile)   rc += db_to_file(file, output, errors);
    output << '\n';
  }
  return rc;
}

//============================================================================

int print_range_sets(InputFile& file,
                     ostream& output)
{
  file.print_range_sets(output);
  return 0;
}

int print_file_index(InputFile& file,
                     ostream& output)
{
  file.print_file_index(output);
  return 0;
}

int db_to_file(InputFile & file,
               ostream& output,
               ostream& errors)
{
  TFile* current_file = file.tfile();
  std::string const& rootFileName = current_file->GetName();
  std::string::size_type const dist = rootFileName.find(".root")-rootFileName.find_last_of('/');
  std::string const base = rootFileName.substr(rootFileName.find_last_of('/')+1, dist);
  std::string const extFileName = base + "db";
  art::SQLite3Wrapper db {current_file, "RootFileDB"};
  int const rc = dbToFile(db, extFileName.c_str());
  if (rc == 0) {
    output << "\nRootFileDB from file \"" << current_file->GetName() << "\"\n"
           << "saved to external database file \"" << extFileName << "\".\n";
  }
  else {
    errors << "\nCould not save RootFileDB from file \"" << current_file->GetName() << "\"\n"
           << "to external database file.\n";
  }
  return rc;
}
