#ifndef test_Integration_oldtest_TestRunSubRunSource_h
#define test_Integration_oldtest_TestRunSubRunSource_h

/*----------------------------------------------------------------------


This source is intended only for test purposes.  With it one can
create data files with arbitrary sequences of run number, subRun
number, and event number in the auxiliary objects in the run tree,
subRun tree, and event tree.  It is quite possible to create an illegal
format that cannot be read with any input module using this source.

The output files of jobs using this source will be used in tests of
input modules to verify they are behaving properly.

The configuration looks as follows

  source = TestRunSubRunSource {
    untracked vint32 runSubRunEvent = { 1, 1, 1,    # run
                                      1, 1, 1,    # subRun
                                      1, 1, 1,    # event
                                      1, 1, 2,    # event
                                      0, 0, 0,    # causes end subRun
                                      0, 0, 0     # causes end run
                                    }
  }

Each line contains 3 values: run, subRun, and event.  These lines
are used in order, one line per each call to readRun_,
readSubRun_, and readEvent, in the order called by the
event processor.  Note that when readRun_ is called only the run
number is used and the other two values are extraneous.  When
readSubRun is called only the first two values are used.
(0, 0, 0) will trigger the end of the current subRun,
run, or job as appropriate for when it appears. Running off the
bottom list is also equivalent to (0,0,0). What is shown above
is the typical sequence one would expect for two events, but this
source is capable of handling arbitrary sequences of run numbers,
subRun number, and events.  For test purposes one can even
include sequences that make no sense and the entries will get
written to the output file anyway.

----------------------------------------------------------------------*/


#include "art/Framework/Core/InputSource.h"

#include "boost/shared_ptr.hpp"
#include <memory>
#include <vector>

namespace art {

  class ParameterSet;
  class InputSourceDescription;
  class EventPrincipal;
  class SubRunPrincipal;
  class RunPrincipal;

  class TestRunSubRunSource : public InputSource {
  public:
    explicit TestRunSubRunSource(ParameterSet const& pset, InputSourceDescription const& desc);
    virtual ~TestRunSubRunSource();

  private:

    virtual ItemType getNextItemType();
    virtual std::auto_ptr<EventPrincipal> readEvent_();
    boost::shared_ptr<SubRunPrincipal> readSubRun_();
    boost::shared_ptr<RunPrincipal> readRun_();

    // This vector holds 3 values representing (run, subRun, event)
    // repeated over and over again, in one vector.
    // Each set of 3 values is placed in the the auxiliary
    // object of the principal returned by a call
    // to readEvent_, readSubRun_, or readRun_.
    // Each set of 3 values is used in the order it appears in the vector.
    // (0, 0, 0) is a special value indicating the read
    // function should return a NULL value indicating last event,
    // last subRun, or last run.
    std::vector<int> runSubRunEvent_;
    std::vector<int>::size_type currentIndex_;
    bool firstTime_;
  };
}
#endif /* test_Integration_oldtest_TestRunSubRunSource_h */

// Local Variables:
// mode: c++
// End:
