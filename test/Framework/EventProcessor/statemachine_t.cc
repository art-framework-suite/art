/*
   -------------------------------------------------------------------
   Test of the statemachine classes.
   -------------------------------------------------------------------
*/

#include "art/Framework/EventProcessor/EPStates.h"
#include "art/Framework/Core/IEventProcessor.h"
#include "test/Framework/EventProcessor/MockEventProcessor.h"

#include "boost/program_options.hpp"

#include <string>
#include <iostream>
#include <fstream>

using namespace statemachine;
using namespace std;

namespace {
  struct HandleEmpty {
    bool runs;
    bool subruns;
  };
}

int main(int argc, char* argv[]) {
  cout << "Running test in statemachine_t.cc\n";

  // Handle the command line arguments
  string inputFile;
  string outputFile;
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "produce help message")
    ("inputFile,i", boost::program_options::value<string>(&inputFile))
    ("outputFile,o", boost::program_options::value<string>(&outputFile))
    ("skipmode,m", "NOMERGE only");
  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);
  if (vm.count("help")) {
    cout << desc << "\n";
    return 1;
  }

  // Get some fake data from an input file.
  // The fake data has the format of a series pairs of items.
  // The first is a letter to indicate the data type
  // r for run, l for subRun, e for event, f for file, s for stop
  // The second item is the run number or subRun number
  // for the run and subRun cases.  For the other cases the number
  // is not not used.
  string mockData;
  ifstream input {inputFile.c_str()};
  for (string line; getline(input, line); )
    mockData += " "+line;

  ofstream output{outputFile.c_str()};

  vector<HandleEmpty> const handleEmptyRunsSubruns = { {false, false},
                                                       {false, true },
                                                       {true,  false},
                                                       {true,  true } };

  for (auto handleEmpty : handleEmptyRunsSubruns) {
    output << "Machine parameters:"
           << "  handleEmptyRuns = " << handleEmpty.runs
           << "  handleEmptySubRuns = " << handleEmpty.subruns << '\n';

    art::MockEventProcessor mockEventProcessor{mockData,
                                               output,
                                               handleEmpty.runs,
                                               handleEmpty.subruns};
    mockEventProcessor.runToCompletion();
    output << '\n';
  }


  return 0;
}
