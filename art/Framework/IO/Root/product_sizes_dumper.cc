// Main program for a utility that looks at an art format event-data file
// and reports on on which objects use how much disk space.
//
// Original author: Rob Kutschke

#include "art/Framework/IO/Root/RootSizeOnDisk.h"
#include "boost/program_options.hpp"
#include "cetlib/container_algorithms.h"

#include "TError.h"
#include "TFile.h"

#include <iostream>
#include <memory>
#include <string>

namespace bpo = boost::program_options;
typedef std::vector<std::string> stringvec;

int
main(int argc, char** argv)
{

  // Parse and validate command line arguments.
  // ------------------
  // use the boost command line option processing library to help out
  // with command line options
  std::ostringstream descstr;
  descstr << argv[0] << " <options> [<source-file>]+";
  bpo::options_description desc(descstr.str());
  desc.add_options()("help,h", "this help message.")(
    "fraction,f",
    bpo::value<double>()->default_value(0.05, "0.05"),
    "floating point number on the range [0,1].  "
    "If a TTree occupies a fraction on disk of the total space in the file "
    "that is less than <f>, then a detailed analysis of its branches will "
    "not be done")(
    "source,s", bpo::value<stringvec>(), "source data file (multiple OK)");
  bpo::options_description all_opts("All Options.");
  all_opts.add(desc);
  // Each non-option argument is interpreted as the name of a file to be
  // processed. Any number of filenames is allowed.
  bpo::positional_options_description pd;
  pd.add("source", -1);
  // The variables_map contains the actual program options.
  bpo::variables_map vm;
  try {
    bpo::store(bpo::command_line_parser(argc, argv)
                 .options(all_opts)
                 .positional(pd)
                 .run(),
               vm);
    bpo::notify(vm);
  }
  catch (bpo::error const& e) {
    std::cerr << "Exception from command line processing in " << argv[0] << ": "
              << e.what() << "\n";
    return 2;
  }
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  // Get the desired minimum fraction
  double minimumFraction(-1.);
  if (vm.count("fraction")) {
    minimumFraction = vm["fraction"].as<double>();
    if (minimumFraction < 0 || minimumFraction > 1) {
      std::cerr << "Must choose fraction between 0 and 1\n"
                << "For usage and options list, please do "
                   "'product_sizes_dumper --help'.\n";
      return 4;
    }
  }

  // Get the names of the files we will process.
  stringvec file_names;
  size_t const file_count = vm.count("source");
  if (file_count < 1) {
    std::cerr << "One or more input files must be specified;"
              << " supply filenames as program arguments\n"
              << "For usage and options list, please do 'product_sizes_dumper "
                 "--help'.\n";
    return 3;
  }
  file_names.reserve(file_count);
  cet::copy_all(vm["source"].as<stringvec>(), std::back_inserter(file_names));

  //=================================================================
  // Separates the output for multiple files.
  std::string const separator = "\n" + std::string(60, '=') + "\n";

  int n(-1);
  for (auto const& filename : file_names) {
    ++n;

    if (file_names.size() > 1 && n != 0)
      std::cout << separator << std::endl;

    // Suppress warnings messages about "no dictionary".
    // This is a little dangerous since it might suppress other warnings too ...
    int errorSave = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kError;
    auto file = std::make_unique<TFile>(filename.c_str());
    gErrorIgnoreLevel = errorSave;

    // Extract and print the information.
    art::RootSizeOnDisk info(filename, file.get());
    file->Close();
    info.print(std::cout, minimumFraction);

    // Mark end of output for this file.
    if (file_names.size() > 1)
      std::cout << "Done: " << filename << std::endl;
  }

  return 0;
}
