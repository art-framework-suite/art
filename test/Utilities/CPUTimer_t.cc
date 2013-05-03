#define _GLIBCXX_USE_NANOSLEEP 1
#define BOOST_TEST_MODULE ( CPUTimer_t )
#include "boost/test/auto_unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"

#include "art/Utilities/CPUTimer.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <sys/resource.h>

struct CPUTimerTtestFixture {
  art::CPUTimer &timer() {
    static art::CPUTimer t_s;
    return t_s;
  }
};

//----------------------------------------------------------------------
// Constants used for controlling the tests of the CPUTimer.
//

// The smallest amount of CPU time we can reliably measure, from two
// consecutive calls to the timer. Time in seconds.
double const small_cputime = 1.5e-3;

// The smallest amount of real (wallclock) time we can reliably measure,
// from two consecutive calls to the timer. Time in seconds.
double const small_realtime = 1.5e-3;


// Return the difference between the total time (user+system) in the two
// rusage structs.
inline double
time_diff(rusage const& a, rusage const& b)
{
  double const sec      = (a.ru_utime.tv_sec  - b.ru_utime.tv_sec)
    +                     (a.ru_stime.tv_sec  - b.ru_stime.tv_sec);
  double const microsec = (a.ru_utime.tv_usec - b.ru_utime.tv_usec)
    +                     (a.ru_stime.tv_usec - b.ru_stime.tv_usec);
  return sec + 1e-6 * microsec;
}

// Make a busy-loop for 'dur' seconds.
double busy_loop(double dur)
{
  double x = 3.14;
  rusage start_time, ru;
  getrusage(RUSAGE_SELF, &start_time) ;
  do {
   for (int i = 0; i < 1000; ++i) x = sin(x);
    getrusage(RUSAGE_SELF, &ru);
  }
  while (time_diff(ru, start_time) < dur);
  return x;
}

//----------------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(CPUTimer_t,CPUTimerTtestFixture)

BOOST_AUTO_TEST_CASE(init)
{
  // A newly-constructed timer should have both realTime and cpuTime of
  // zero.
  BOOST_CHECK_EQUAL(timer().realTime(), 0.0);
  BOOST_CHECK_EQUAL(timer().cpuTime(), 0.0);
}

BOOST_AUTO_TEST_CASE(timer1)
{
  // Run the timer while we sleep, then stop. This should use clock
  // time, but little CPU time.
  timer().stop();
  timer().reset();
  timer().start();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  timer().stop();

  BOOST_CHECK_CLOSE(timer().realTime(), 0.050, 5.0); // difference in percentage
  std::cout << "timer1 cpu: " << timer().cpuTime() << " real: " << timer().realTime() << std::endl;
  BOOST_CHECK_SMALL(timer().cpuTime(), small_cputime);
}

BOOST_AUTO_TEST_CASE(timer2)
{
  // Run the time while we sleep. Capture the time without stopping the
  // timer. This should use clock time but little CPU time.
  timer().stop();
  timer().reset();
  timer().start();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  // We have to capture the times before we do comparisons, or else
  // we'll have used too much CPU time.
  double const cpu = timer().cpuTime();
  double const real = timer().realTime();

  BOOST_CHECK_CLOSE(real, 0.050, 5.0); // difference in percentage
  BOOST_CHECK_SMALL(cpu, small_cputime);
}

BOOST_AUTO_TEST_CASE(nullStart)
{
  // Get the time immediately after starting. This should give close to
  // zero for both clock and CPU time.
  timer().stop();
  timer().reset();
  timer().start();

  // Test
  BOOST_CHECK_SMALL(timer().realTime(), small_realtime);
  BOOST_CHECK_SMALL(timer().cpuTime(), small_cputime);
}

BOOST_AUTO_TEST_CASE(doubleStop)
{
  // Make sure the time between two stop() calls without an intervening
  // start() or reset() does not change.
  timer().stop();
  timer().reset();
  timer().start();
  double const dur = 0.150; // seconds
  std::cout << busy_loop(dur) << "\n";
  timer().stop();
  double real = timer().realTime();
  double cpu = timer().cpuTime();
  timer().stop();

  BOOST_CHECK_EQUAL(timer().realTime(), real);
  BOOST_CHECK_EQUAL(timer().cpuTime(),  cpu);
}

BOOST_AUTO_TEST_CASE(reset)
{
  // Make sure reset() zeros both real and CPU time.
  timer().start();
  double const dur = 0.150;
  std::cout << busy_loop(dur) << "\n";
  timer().stop();
  BOOST_CHECK_GT(timer().realTime(), 0.0);
  BOOST_CHECK_GT(timer().cpuTime(), 0.0);

  timer().reset();
  BOOST_CHECK_EQUAL(timer().realTime(), 0.0);
  BOOST_CHECK_EQUAL(timer().cpuTime(), 0.0);
}


BOOST_AUTO_TEST_CASE(checkUsage)
{
  timer().stop();
  timer().reset();
  timer().start();

  double const dur = 0.135;
  std::cout << busy_loop(dur) << "\n";
  timer().stop();

  BOOST_CHECK_CLOSE(timer().realTime(), dur, 5.0);
  BOOST_CHECK_CLOSE(timer().cpuTime(), dur, 5.0);
}

BOOST_AUTO_TEST_SUITE_END()
