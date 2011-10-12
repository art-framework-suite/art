#define BOOST_TEST_MODULE ( CPUTimer_t )
#include "boost/test/auto_unit_test.hpp"

#include "art/Utilities/CPUTimer.h"

#include <iostream>
#include <string>
extern "C" {
#include <unistd.h>
#include <sys/resource.h>
}

struct CPUTimerTtestFixture {
  art::CPUTimer &timer() {
    static art::CPUTimer t_s;
    return t_s;
  }
};

BOOST_FIXTURE_TEST_SUITE(CPUTimer_t,CPUTimerTtestFixture)

  BOOST_AUTO_TEST_CASE(init)
{
  BOOST_CHECK_EQUAL(timer().realTime(),0.0);
  BOOST_CHECK_EQUAL(timer().cpuTime(),0.0);
}

BOOST_AUTO_TEST_CASE(timer1)
{
  timer().start();
  sleep(2);
  timer().stop();
  BOOST_CHECK_GT(timer().realTime(),2.0);
  BOOST_CHECK_LE(timer().cpuTime()+1.5,timer().realTime()); // Allow for timer slop.
}

BOOST_AUTO_TEST_CASE(timer2)
{
  timer().start();
  sleep(2);
  BOOST_CHECK_GT(timer().realTime(),4.0);
}

BOOST_AUTO_TEST_CASE(nullStart)
{
  //this should do nothing
  timer().start();
  BOOST_CHECK_GT(timer().realTime(),4.0);
}

BOOST_AUTO_TEST_CASE(doubleStop)
{
  sleep(2);
  timer().stop();
  double real = timer().realTime();
  double cpu = timer().cpuTime();

  //this should do nothing
  timer().stop();
  BOOST_CHECK_EQUAL(timer().realTime(),real);
  BOOST_CHECK_EQUAL(timer().cpuTime(),cpu);
}

BOOST_AUTO_TEST_CASE(reset)
{
  timer().reset();
  BOOST_CHECK_EQUAL(timer().realTime(),0.0);
  BOOST_CHECK_EQUAL(timer().cpuTime(),0.0);
}

BOOST_AUTO_TEST_CASE(checkUsage)
{
  rusage theUsage;
  getrusage(RUSAGE_SELF, &theUsage) ;
  struct timeval startTime;
  startTime.tv_sec =theUsage.ru_utime.tv_sec;
  startTime.tv_usec =theUsage.ru_utime.tv_usec;

  timer().start();
  struct timeval nowTime;
  do {
    rusage theUsage2;
    getrusage(RUSAGE_SELF, &theUsage2) ;
    nowTime.tv_sec =theUsage2.ru_utime.tv_sec;
    nowTime.tv_usec =theUsage2.ru_utime.tv_usec;
  }while(nowTime.tv_sec -startTime.tv_sec +1E-6*(nowTime.tv_usec-startTime.tv_usec) <1);
  timer().stop();

  BOOST_CHECK_GE(timer().realTime(),1.0);
  BOOST_CHECK_GE(timer().cpuTime(),1.0);
}

BOOST_AUTO_TEST_SUITE_END()
