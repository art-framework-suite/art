/*----------------------------------------------------------------------

Test of the statemachine classes.



----------------------------------------------------------------------*/

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

  ostream& operator<<(ostream& os, FileMode const fileMode)
  {
    if (fileMode == NOMERGE) os << "NOMERGE";
    else if (fileMode == MERGE) os << "MERGE";
    else if (fileMode == FULLLUMIMERGE) os << "FULLLUMIMERGE";
    else os << "FULLMERGE";
    return os;
  }
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
    ("skipmode,m", "NOMERGE, FULLLUMIMERGE and FULLMERGE only")
    ("skipmodes,s", "NOMERGE and FULLMERGE only");
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
  // is not not used.  This series of fake data items is terminated
  // by a period (blank space and newlines are ignored).
  string mockData;
  ifstream input {inputFile.c_str()};
  for (string line; getline(input, line); )
    mockData += " "+line;

  ofstream output{outputFile.c_str()};

  vector<FileMode> fileModes;
  fileModes.reserve(4);
  fileModes.push_back(NOMERGE);
  if (!vm.count("skipmode") && !vm.count("skipmodes")) {
    fileModes.push_back(MERGE);
  }
  if (!vm.count("skipmodes")) {
    fileModes.push_back(FULLLUMIMERGE);
  }
  fileModes.push_back(FULLMERGE);

                                                     // Runs   Subruns
  vector<HandleEmpty> const handleEmptyRunsSubruns = { {false, false   },
                                                       {false, true    },
                                                       {true,  false   },
                                                       {true,  true    } };

  for (auto fileMode : fileModes) {
    for (auto handleEmpty : handleEmptyRunsSubruns ) {
      output << "\nMachine parameters:"
             << "  mode = " << fileMode
             << "  handleEmptyRuns = " << handleEmpty.runs
             << "  handleEmptySubRuns = " << handleEmpty.subruns << '\n';

      art::MockEventProcessor mockEventProcessor(mockData,
                                                 output,
                                                 fileMode,
                                                 handleEmpty.runs,
                                                 handleEmpty.subruns);
      mockEventProcessor.runToCompletion();
    }
  }


  return 0;
}
