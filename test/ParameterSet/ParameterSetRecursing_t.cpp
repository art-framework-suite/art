#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <assert.h>

#include "art/Utilities/EDMException.h"
#include "art/Persistency/Provenance/ParameterSetID.h"
#include "art/ParameterSet/ParameterSet.h"

using art::ParameterSet;
using art::ParameterSetID;
using std::set;
using std::string;
using std::vector;

void work()
{
  set<ParameterSetID> all_ids;
  ParameterSet level1;
  level1.addParameter<int>("level", 1);

  ParameterSet level2;
  level2.addParameter<int>("level", 2);
  all_ids.insert(level2.id());
  level1.addParameter<ParameterSet>("pset2", level2);


  vector<ParameterSet> v1(2);
  v1[0].addParameter<int>("slot", 0);
  all_ids.insert(v1[0].id());
  v1[1].addParameter<int>("slot", 1);
  level1.addParameter("vpset1", v1);
  all_ids.insert(v1[1].id());
  all_ids.insert(level1.id());

  vector<std::string> pset_names, vpset_names;

  const bool tracked = true;
  // const bool untracked = false; // Not used. Causes compiler warning.
  size_t n_psets = level1.getParameterSetNames(pset_names, tracked);
  size_t n_vpsets = level1.getParameterSetVectorNames(vpset_names, tracked);
  assert( n_psets == 1 );
  assert( pset_names[0] == "pset2" );
  {
    ParameterSet pset2 = level1.getParameter<ParameterSet>(pset_names[0]);
    assert( pset2.getParameter<int>("level") == 2);
  }

  assert( n_vpsets == 1 );
  assert( vpset_names[0] == "vpset1" );
  {
    vector<ParameterSet> vpset1 =
      level1.getParameter<vector<ParameterSet> >(vpset_names[0]);
    assert( vpset1.size() == 2 );
    assert( vpset1[0].getParameter<int>("slot") == 0);
    assert( vpset1[1].getParameter<int>("slot") == 1);

  }
  vector<ParameterSet> exploded;
  art::pset::explode(level1, exploded);
  assert( exploded.size() == all_ids.size() );
  set<ParameterSetID> exploded_ids;
  // This could be written with a call to transform (and using bind
  // and a lambda function)... but this is less obfuscated!
  for (size_t i = 0; i < exploded.size(); ++i)
    {
      exploded_ids.insert(exploded[i].id());
    }
  assert( exploded_ids == all_ids );
}

void work2()
{
  art::ParameterSet empty;
  assert( empty.empty() );
  std::vector<art::ParameterSet> exploded;
  art::pset::explode(empty, exploded);
  assert( exploded.size() == 1 );
  assert( exploded[0].empty() );
}

int main()
{
  int rc = 1;
  try
    {
      work();
      work2();
      rc = 0;
    }
  catch ( art::Exception& x)
    {
      std::cout << "Caught an art::Exception:\n"
		<< x
		<< '\n';
    }
  catch ( ... )
    {
      std::cout << "Caught an unidentified exception\n";
    }
  return rc;

}
