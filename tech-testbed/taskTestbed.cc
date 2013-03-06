#define BOOST_TEST_ALTERNATIVE_INIT_API
#include "art/Utilities/quiet_unit_test.hpp"
using namespace boost::unit_test;

#include "tech-testbed/EventQueue.hh"
#include "tech-testbed/SerialTaskQueue.hh"
#include "tech-testbed/make_reader.hh"

#include "tbb/task_scheduler_init.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include "boost/program_options.hpp"

namespace buf = boost::unit_test::framework;
namespace bpo = boost::program_options;

class TaskTestbed {
public:
  TaskTestbed(unsigned short nThreads);
  void operator() (size_t nSchedules,
                   size_t nEvents);

private:
  tbb::task_scheduler_init tbbManager_;
};

// Actual working function
void exec_taskTestbed(unsigned short nThreads,
                      size_t perSchedule, std::vector<size_t> const & runs) {
  TaskTestbed work(nThreads);
  for (auto const ns : runs) {
    work(ns, ns * perSchedule);
  }
}

bool
init_unit_test_suite()
{
  auto & mts = boost::unit_test::framework::master_test_suite();
  std::ostringstream descstr;
  descstr << "Usage: "
          << mts.argv[0] << "-h|--help\n"
          << "       "
          << mts.argv[0]
          << "[<options>] <nSchedules>+\n\nOptions";
  bpo::options_description visible(descstr.str()), all;

  std::vector<size_t> runs;
  visible.add_options()
    ("help,h", "Show help.")
    ("threads",
     bpo::value<unsigned short>()->
     default_value(tbb::task_scheduler_init::default_num_threads(),
                   "No. of cores"),
     "No. of TBB threads.")
    ("nevt",
     bpo::value<size_t>()->default_value(4000, "4000"),
     "Number of events per schedule.")
    ;
  all.add(visible).add_options()("nSchedules",
                                 bpo::value<std::vector<size_t> >(&runs),
                                 "Number of schedules.");
  bpo::positional_options_description pd;
  pd.add("nSchedules", -1);
  // Parse the command line.
  bpo::variables_map vm;
  try {
    bpo::store(bpo::command_line_parser(mts.argc, mts.argv).
               options(all).
               positional(pd).
               run(),
               vm);
    bpo::notify(vm);
  }
  catch (bpo::error const & e) {
    std::cerr << "Exception from command line processing in " << mts.argv[0]
              << ": " << e.what() << "\n";
    return false;
  }

  if (vm.count("help") > 0) {
    std::cerr << visible;
    return false;
  }

  if (vm.count("nSchedules") == 0) {
    std::cerr << "Exception from command line processing in " << mts.argv[0]
              << ": "
              << " Required non-option argument <nSchedules>+ missing.\n";
    return false;
  }
  tbb::task_scheduler_init tbbManager(vm["threads"].as<unsigned short>());
  auto nevt = vm["nevt"].as<size_t>();
  framework::master_test_suite().
    add(BOOST_TEST_CASE(std::bind(&exec_taskTestbed,
                                  vm["threads"].as<unsigned short>(),
                                  nevt,
                                  runs)));
  return true;
}

int main(int argc, char *argv[]) {
  char const *cmp = "-c";
  char to[] = "--config";
  for (int i {0};
       i < argc;
       ++i) {
    if (strncmp(cmp, argv[i], 2) == 0) {
      argv[i] = to;
      break; // Done.
    }
  }
  return unit_test_main(&init_unit_test_suite, argc, argv);
}

TaskTestbed::TaskTestbed(unsigned short nThreads)
:
  tbbManager_(nThreads)
{
}

void
TaskTestbed::
operator() (size_t nSchedules,
            size_t nEvents)
{

  demo::SerialTaskQueue sQ;

  // Dummy task (never spawned) to handle all the others as children.
  std::shared_ptr<tbb::task> topTask
  { new (tbb::task::allocate_root()) tbb::empty_task {},
      [](tbb::task* iTask){tbb::task::destroy(*iTask);}
  };

  topTask->set_ref_count(nSchedules + 1);

  size_t evCounter { nEvents };
  demo::EventQueue eQ;
  for ( size_t i { 0 }; i != evCounter; ++i) {
    eQ.push(std::shared_ptr<demo::EventPrincipal>{new demo::EventPrincipal{i}});
  }
  std::random_device rd;

  std::vector<demo::Schedule> schedules;
  schedules.reserve(nSchedules);
  tbb::tick_count t0 = tbb::tick_count::now();
  for (size_t i { 0 }; i < nSchedules; ++i) {
    eQ.push(std::shared_ptr<demo::EventPrincipal>()); // EOD
    schedules.emplace_back(i, rd(), 50000);
    sQ.push(demo::make_reader(cet::exempt_ptr<demo::Schedule>(&schedules.back()),
                              topTask.get(),
                              sQ,
                              eQ));
  }

  // Wait for all tasks to complete.
  topTask->wait_for_all();
  tbb::tick_count t1 = tbb::tick_count::now();

  size_t totalEvents = 0;
  tbb::tick_count::interval_t totalTime(0.0);
  std::ofstream file("taskTestbed.dat");
  demo::Schedule::printHeader(file);
  for (auto const & sched : schedules) {
    auto evts = sched.eventsProcessed();
    auto time = sched.timeTaken();
    std::cout << "Schedule " << sched.id()
              << " processed "
              << sched.itemsGenerated() << " items from "
              << evts << " events ("
              << sched.itemsGenerated() / static_cast<double>(evts)
              << "/event) in "
              << time.seconds() * 1000 << " ms."
              << std::endl;
    totalEvents += evts;
    totalTime += time;
    file << sched;
  }

  auto elapsed_time = (t1 -t0).seconds();
  std::cout << "Processed a total of "
            << totalEvents << " events using "
            << nSchedules << " schedules in "
            << totalTime.seconds() << " s CPU, "
            << elapsed_time << " s elapsed (speedup x"
            << totalTime.seconds() / elapsed_time << ")."
            << std::endl;

  BOOST_REQUIRE_EQUAL(nEvents, totalEvents);

  return;
}

// Local Variables:
// mode: c++
// End:
