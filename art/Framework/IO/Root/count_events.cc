////////////////////////////////////////////////////////////////////////
// count_events
//
// Use FileIndex to quickly obtain the number of events (and runs, and
// subruns) in the specified files.
////////////////////////////////////////////////////////////////////////

#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Persistency/Provenance/BranchType.h"

#include "boost/program_options.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "TBranch.h"
#include "TError.h"
#include "TFile.h"
#include "TTree.h"

namespace bpo = boost::program_options;

namespace {
  bool want_hr = false;

  using namespace std::string_literals;
  struct pluralize {
    pluralize(std::size_t count,
              std::string thing,
              std::string singular = ""s,
              std::string plural = "s"s)
      :
      msg(std::to_string(count) + ' ' + thing + ((count == 1) ? singular : plural))
      {
      }
    std::string msg;
  };

  std::ostream & operator << (std::ostream & os, pluralize p)
  {
    os << p.msg;
    return os;
  }

  bool count_events(std::string const & fileName, std::ostream & os, std::ostream & err)
  {
    // Ignore less severe warnings for the purposes of opening the file.
    auto savedErrorLevel = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kBreak;
    TFile * tf = TFile::Open(fileName.c_str());
    gErrorIgnoreLevel = savedErrorLevel;
    if (tf == nullptr) {
      err << fileName << "\tCould not be opened by ROOT: skipped.\n";
      return false;
    }
    std::array<std::size_t, art::NumBranchTypes> counters { 0ull };
    art::FileIndex fi;
    for (int i = art::InEvent; i != art::InResults; ++i) {
      std::string treeName = art::BranchTypeToProductTreeName(static_cast<art::BranchType>(i));
      TTree * tree = static_cast<TTree*> (tf->Get(treeName.c_str()));
      if (!tree) {
        err << fileName << "\tNot a valid art ROOT-format file: skipped.\n";
        return false;
      }
      counters[i] = tree->GetEntries();
    }
    {
      auto tree = static_cast<TTree*>(tf->Get(art::BranchTypeToProductTreeName(art::InResults).c_str()));
      if (tree && (tree->GetNbranches() > 1)) {
        counters[art::InResults] = 1;
      }
    }
    if (want_hr) {
      os << fileName << "\t"
         << pluralize(counters[art::InRun], "run") << ", "
         << pluralize(counters[art::InSubRun], "subrun") << ", "
         << pluralize(counters[art::InEvent], "event") << ", and "
         << pluralize(counters[art::InResults], "result") << ".\n";
    } else {
      os << fileName << '\t'
         << counters[art::InRun] << '\t'
         << counters[art::InSubRun] << '\t'
         << counters[art::InEvent] << '\t'
         << counters[art::InResults] << '\n';
    }
    return true;
  }
}

int main(int argc, char ** argv) {
  using stringvec = std::vector<std::string>;
  int result = 1;
  std::ostringstream descstr;
  descstr << argv[0]
          << "Usage: count_events [<options>] <filename>+\n";
  bpo::options_description desc(descstr.str());
  desc.add_options()
    ("hr", "Human-readable output")
    ("help,h", "this help message.")
    ("source,s", bpo::value<stringvec>()->composing(),
     "source data file (multiple OK).");
  bpo::options_description all_opts("All Options.");
  all_opts.add(desc);
  // Each non-option argument is interpreted as the name of a file to be
  // processed. Any number of filenames is allowed.
  bpo::positional_options_description pd;
  pd.add("source", -1);
  // The variables_map contains the actual program options.
  bpo::variables_map vm;
  try {
    bpo::store(bpo::command_line_parser(argc, argv).options(all_opts).positional(pd).run(),
               vm);
    bpo::notify(vm);
  }
  catch (bpo::error const & e) {
    std::cerr << "Exception from command line processing in "
              << argv[0] << ": " << e.what() << "\n";
    return 2;
  }
  if (vm.count("help")) {
    std::cerr << desc << std::endl;
    return 1;
  } else if (vm.count("hr")) {
    want_hr = true;
  }
  if (vm.count("source") == 0) {
    std::cerr << "Require at least one source file.\n";
    std::cerr << desc << "\n";
    return 1;
  }
  auto const & sources = vm["source"].as<stringvec>();
  auto const expected = sources.size();
  auto succeeded =
    std::count_if(sources.cbegin(),
                  sources.cend(),
                  std::bind(&count_events, std::placeholders::_1,
                            std::ref(std::cout),
                            std::ref(std::cerr)));
  if (expected == static_cast<size_t>(succeeded))
  {
    std::cout << "Counted events successfully for "
              << expected
              << " specified files."
              << std::endl;
    result = 0;
  } else {
    result = 1;
    std::cout << "Failed to count events for "
              << expected - succeeded
              << " of "
              << expected
              << " specified files."
              << std::endl;
  }
  return result & 0xff;
}
