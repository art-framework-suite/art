//----------------------------------------------------------------------
//
// This program tests the behavior of untracked parameters in
// ParameterSet objects.
//
//----------------------------------------------------------------------
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "art/Persistency/Provenance/ParameterSetID.h"
#include "art/Utilities/EDMException.h"
#include "art/ParameterSet/ParameterSet.h"
#include "art/ParameterSet/MakeParameterSets.h"

using edm::ParameterSetID;
using edm::ParameterSet;
using edm::InputTag;
typedef std::vector<edm::ParameterSet> VPSet;


void testUntrackedInternal()
{
  {
    // Make sure that when we reconstitute a ParameterSet from the
    // string made by toStringOfTracked() that we only obtain the
    // tracked parameters.
    ParameterSet inner;
    inner.addParameter<int>("innerint", 100);
    inner.addUntrackedParameter<int>("innerint_untracked", 200);

    ParameterSet inner_tracked;
    inner_tracked.addParameter<int>("innerint", 100);

    VPSet inner_vpset;
    inner_vpset.push_back(inner);
    inner_vpset.push_back(inner);
    inner_vpset.push_back(inner);

    ParameterSet p1;

    p1.addParameter<VPSet>("vpset1", inner_vpset);
    p1.addUntrackedParameter<VPSet>("vpset2", inner_vpset);
    p1.addUntrackedParameter<int>("i", 2);
    p1.addUntrackedParameter<std::string>("s", "xyz");
    p1.addParameter<int>("j", 2112);
    p1.addParameter<ParameterSet>("ps", inner);
    p1.addUntrackedParameter<ParameterSet>("ups", inner);
    assert( p1.getUntrackedParameter<int>("i",10) == 2);
    assert( p1.getParameter<int>("j") == 2112);
    assert( p1.getUntrackedParameter<std::string>("s", "abc") == "xyz");
    assert( p1.getParameter<VPSet>("vpset1") == inner_vpset );
    assert( p1.getUntrackedParameter<VPSet>("vpset2", VPSet())
	    == inner_vpset);
    std::string p1_whole = p1.toString();
    std::string p1_rep = p1.toStringOfTracked();
    std::cerr << "Whole string\n"
	      << p1_whole
	      << "\ntracked part\n"
	      << p1_rep
	      << '\n';
    ParameterSet p2(p1_rep);
    assert( p2.getUntrackedParameter<int>("i",10) == 10);
    assert( p2.getParameter<int>("j") == 2112);
    assert( p2.getUntrackedParameter<std::string>("s", "abc") == "abc");
    ParameterSet recovered_inner = p2.getParameter<ParameterSet>("ps");
    assert( recovered_inner == inner_tracked );
    assert( recovered_inner.getParameter<int>("innerint") == 100);

    VPSet recovered_inner_vpset = p2.getParameter<VPSet>("vpset1");
    assert( recovered_inner_vpset.size() == 3 );
    {
      VPSet::const_iterator i = recovered_inner_vpset.begin();
      VPSet::const_iterator e = recovered_inner_vpset.end();
      for ( ; i!=e; ++i )
	{
	  assert( i->getParameter<int>("innerint") == 100 );
	  assert( i->getUntrackedParameter<int>("innerint_untracked", 1)
		  == 1 ); // not 100, which was the untracked original.
	}
    }

    // Make sure we don't recover an untracked parameter in nested
    // ParameterSets
    assert( recovered_inner.getUntrackedParameter<int>("innerint_untracked"
						       ,100)
	    == 100);

    assert( p2.getUntrackedParameter<ParameterSet>("ups",
						   ParameterSet()) ==
	    ParameterSet());

    assert( p1 == p2 );
  }
  ParameterSet p;
  assert( p.empty() );
  ParameterSetID empty_id = p.id();

  // ParameterSet should not longer be empty after adding an untracked
  // parameter.
  p.addUntrackedParameter<int>("i1", 2112);
  assert( !p.empty());

  // Adding an untracked parameter should not have changed the ID.
  assert( p.id() == empty_id );

  // We should find a parameter named 'i1'
  assert( p.getUntrackedParameter<int>("i1", 10) == 2112 );

  // We should not find one named 'nonesuch'.
  assert( p.getUntrackedParameter<int>("nonesuch", 1) == 1 );

  // If we grab out only the tracked parameters, we should be back to
  // the empty ParameterSet.
  ParameterSet also_empty = p.trackedPart();
  assert( also_empty.getUntrackedParameter<int>("i1", 10) == 10 );


  // We should not find a string named "i1", nor should we get back
  // our default, because we have an "i1" that is of a different type.
  try
    {
      (void)  p.getUntrackedParameter<std::string>("i1", "");
      assert( "Failed to throw required exception" == 0 );
    }
  catch ( edm::Exception& x)
    {
      // This is the expected exception
      assert( x.categoryCode() == edm::errors::Configuration);
    }
  catch (...)
    {
      assert("Threw the wrong kind of exception" == 0);
    }

  //
  try
    {
      (void)p.getParameter<int>("i2");
      assert( "Failed to throw required exception when getting i2" == 0);
    }
  catch ( edm::Exception& x)
    {
      // This is the expected exception..
      assert( x.categoryCode() == edm::errors::Configuration );
    }
  catch ( ... )
    {
      assert( "Threw the wrong exception when getting i2" == 0);
    }
  InputTag tag("label");
  InputTag tag2("label:instance");
  InputTag tag2b("label:instance");
  InputTag tag3("label::process");
  InputTag tag3b("label::process");
  assert(tag2 == tag2b);
  assert(tag3 == tag3b);
  assert(tag.encode() == "label");
  assert(tag2.encode() == "label:instance");
  assert(tag3.encode() == "label::process");
}

void testUntrackedFromScript()
{
  std::string filetext =

  boost::shared_ptr<ParameterSet> mainps;
  "import FWCore.ParameterSet.Config as cms\n"
  "process = cms.Process('TEST')\n"
  "process.source = cms.Source('DummySource')\n"
  "process.p1 = cms.PSet(\n"
  "    a = cms.untracked.int32(6)\n"
  ")\n"
  "process.p2 = cms.untracked.PSet(\n"
  "    b = cms.int32(10)\n"
  ")\n"
  "process.p3 = cms.untracked.PSet(\n"
  "    d = cms.untracked.double(1.5)\n"
  ")\n"
  "process.vp = cms.untracked.VPSet(cms.PSet(\n"
  "    i = cms.int32(1)\n"
  "), \n"
  "    cms.PSet(\n"
  "        f = cms.untracked.double(1.5)\n"
  "    ))\n";

  boost::shared_ptr<ParameterSet> mainps = PythonProcessDesc(filetext).getProcessPSet();
  //edm::makeParameterSets(filetext, mainps, services);

  ParameterSet p1 = mainps->getParameter<ParameterSet>("p1");
  assert( p1.getUntrackedParameter<int>("a")  == 6 );

  ParameterSet p2 = mainps->getUntrackedParameter<ParameterSet>("p2");
  assert( p2.getParameter<int>("b") == 10 );

  ParameterSet p3 = mainps->getUntrackedParameter<ParameterSet>("p3");
  assert( p3.getUntrackedParameter<double>("d") == 1.5);

  VPSet vp = mainps->getUntrackedParameter<VPSet >("vp", VPSet());
  assert( vp.size() == 2 );
  assert (vp[0].getParameter<int>("i") == 1);
  assert( vp[1].getUntrackedParameter<double>("f", 2.0) == 1.5);
}

void testTracked()
{

  // If we look for an 'untracked parameter', when we have a tracked
  // one, we should get an exception throw.
  ParameterSet p;
  ParameterSetID empty_id = p.id();

  p.addParameter<int>("a", 3);
  // Adding a tracked parameter should change the ID.
  assert( p.id() != empty_id );
  try
    {
      (void)p.getUntrackedParameter<int>("a", 10);
      assert ("Failed to throw the required exception" == 0);
    }
  catch ( edm::Exception& x )
    {
      // This is the expected exception
      assert (x.categoryCode() == edm::errors::Configuration);
    }
  catch (...)
    {
      assert ("Threw the wrong exception getting a" == 0);
    }

}


int main()
{
  int rc = 1;
  try
    {
      testUntrackedInternal();
      testUntrackedFromScript();
      testTracked();
      rc = 0;
    }
  catch ( edm::Exception& x)
    {
      std::cerr << "edm::Exception caught\n"
		<< x
		<< '\n';
      rc = -1;
    }
  catch ( ... )
    {
      std::cerr << "Unknown exception caught\n";
      rc = -2;
    }
  return rc;
}
