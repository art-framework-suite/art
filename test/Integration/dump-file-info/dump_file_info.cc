// dump_file_info.cc

#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/detail/getFileContributors.h"
#include "art/Persistency/RootDB/SQLite3Wrapper.h"
#include "art/Persistency/RootDB/tkeyvfs.h"
#include "boost/program_options.hpp"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "cetlib/container_algorithms.h"
#include "test/Integration/dump-file-info/InputFile.h"

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

using art::detail::InputFile;
using std::ostream;
using std::string;
using std::vector;
using stringvec = vector<string>;


int print_range_sets(InputFile& file, ostream& output);
int print_file_index(InputFile& file, ostream& output);

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

  bool const printRangeSets = vm.count("print-range-sets") > 0;
  bool const printFileIndex = vm.count("print-file-index") > 0;

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

  ostream& output = std::cout;

  int rc {0};
  for (auto const& fn : file_names) {
    output << std::string(30,'=') << '\n'
           << "File: " << fn.substr(fn.find_last_of('/')+1ul) << '\n';
    InputFile file {fn};
    if (printRangeSets) rc += print_range_sets(file, output);
    if (printFileIndex) rc += print_file_index(file, output);
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
