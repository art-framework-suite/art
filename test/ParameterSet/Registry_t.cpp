//----------------------------------------------------------------------
//
// This program test the behavior of pset::Registry.
//
//
//----------------------------------------------------------------------
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "boost/thread.hpp"

#include "art/Persistency/Provenance/ParameterSetID.h"
#include "art/Utilities/EDMException.h"
#include "art/ParameterSet/ParameterSet.h"
#include "art/ParameterSet/Registry.h"

struct ThreadWorker
{
  ThreadWorker(int n_to_add, int n_of_lookups, int ofs) :
    number_to_add(n_to_add),
    number_of_lookups(n_of_lookups),
    offset(ofs)
  { }

  void operator()();

  int number_to_add;
  int number_of_lookups;
  int offset;
};

void
ThreadWorker::operator()()
{
  art::pset::Registry* reg = art::pset::Registry::instance();
  assert (reg);

  // Add a bunch of items
  std::vector<art::ParameterSetID> ids_issued;
  std::vector<art::ParameterSet> psets;

  for (int i = offset; i < number_to_add+offset; ++i)
  {
      art::ParameterSet ps;
      ps.addParameter<int>("i", i);
      ps.addUntrackedParameter<double>("d", 2.5);
      ids_issued.push_back(ps.id());
      psets.push_back(ps);
      //reg->insertMapped(ps);
  }
  art::ParameterSet toplevel;
  toplevel.addParameter("guts", psets);
  art::pset::loadAllNestedParameterSets(reg, toplevel);

  art::ParameterSetID topid = art::pset::getProcessParameterSetID(reg);
  assert( topid.isValid() );
  assert( topid == toplevel.id() );

  // Look up items we have just put in.
  typedef std::vector<art::ParameterSetID>::const_iterator iter;
  for (iter i=ids_issued.begin(), e=ids_issued.end(); i!=e; ++i)
  {
      art::ParameterSet ps = getParameterSet(*i);
      assert(ps.id() == *i);
      assert(ps.getParameter<int>("i") > 0);
      double mye = ps.getUntrackedParameter<double>("e", 1.0);
      assert ( std::abs(mye-1.0) < 1.0e-10 );
  }

  for (iter i=ids_issued.begin(), e=ids_issued.end(); i!=e; ++i)
  {
    art::ParameterSet ps;
    assert(reg->getMapped(*i, ps));
    assert(ps.id() == *i);
  }

}

void work()
{
  art::pset::Registry* reg = art::pset::Registry::instance();
  assert (reg);

  // Launch a bunch of threads, which beat on the Registry...
  boost::thread_group threads;
  // don't make this too high, or resource exhaustion may kill
  // the whole test...
  const int NUM_THREADS = 10;
  const int NUM_PSETS_PER_THREAD = 500;
  const int NUM_LOOKUPS_PER_THREAD = 1000;
  for (int i = 0; i < NUM_THREADS; ++i)
    threads.create_thread(ThreadWorker(NUM_PSETS_PER_THREAD,
 				       NUM_LOOKUPS_PER_THREAD,
 				       NUM_PSETS_PER_THREAD));
  threads.join_all();
}

void work2()
{
  // This part is not multi-threaded; it checks only the return value
  // of the insertParameterSet member function.
  art::pset::Registry* reg = art::pset::Registry::instance();

  // Make a new ParameterSet, not like those already in the Registry.
  art::ParameterSet ps;
  std::string value("and now for something completely different...");
  ps.addParameter<std::string>("s", value);

  // First call should insert the new ParameterSet; second call should
  // not.
  assert(reg->insertMapped(ps));
  assert(!reg->insertMapped(ps));

}


int main()
{
  int rc = 1;
  try
  {
      work();
      work2();

      // Look at what we have saved.
      //std::cout << "Here comes the registry..." << std::endl;
      //std::cout << *art::pset::Registry::instance() << std::endl;
      //std::cout << "...done" << std::endl;
      rc = 0;
  }
  catch (art::Exception& x)
  {
      std::cerr << "art::Exception caught\n"
		<< x
		<< '\n';
      rc = -1;
  }
  catch (std::bad_alloc & x)
  {
      std::cerr << "bad alloc: " << x.what() << '\n';
      rc = -2;
  }

  catch (boost::thread_resource_error & x)
  {
      std::cerr << "resource error: " << x.what()
		<< "\nTry decreasing NUM_THREADS, and recompiling\n";
      rc = -3;
  }


  catch (boost::lock_error & x)
  {
      std::cerr << "lock error: " << x.what() << '\n';
      rc = -4;
  }




  catch (...)
  {
      std::cerr << "Unknown exception caught\n";
      rc = 1;
  }
  return rc;
}
