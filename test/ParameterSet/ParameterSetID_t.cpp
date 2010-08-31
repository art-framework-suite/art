//----------------------------------------------------------------------
//
// This program tests the calculation of ParameterSetIDs for
// ParameterSets.
//
//
//----------------------------------------------------------------------
#include <cassert>
#include <iostream>
#include <string>

#include "art/Utilities/EDMException.h"
#include "art/Persistency/Provenance/ParameterSetID.h"
#include "art/ParameterSet/ParameterSet.h"

void work()
{
  edm::ParameterSet p;
  edm::ParameterSetID def_id = p.id();

  // Adding an untracked parameter should not change the id.
  p.addUntrackedParameter<int>("i", 100);
  p.addUntrackedParameter<unsigned int>("j", 20);
  assert ( p.id() == def_id );

  p.addUntrackedParameter("d", 2.5F);

  std::string s("boo");
  std::vector<std::string> empty_vs;
  p.addUntrackedParameter("s", s);
  p.addUntrackedParameter("vs", empty_vs);
  assert ( p.id() == def_id );


  // Adding a tracked parameter *should* change the id
  p.addParameter<double>("d", 1.5);
  assert ( p.id() != def_id );

  // A second parameter set, with only this tracked parameter, should
  // give the same ParameterSetID.
  edm::ParameterSet p2;
  p2.addParameter<double>("d", 1.5);
  assert( p.id() == p2.id() );
}


int main()
{
  int rc = 1;
  try
    {
      work();
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
