
// Test code for the ParameterSetDescription and ParameterDescription
// classes.

#include "art/ParameterSet/ParameterSetDescription.h"
#include "art/ParameterSet/ParameterDescription.h"
#include "art/ParameterSet/ParameterSet.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Utilities/InputTag.h"
#include "art/ParameterSet/FileInPath.h"
#include "art/Utilities/EDMException.h"

#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <boost/cstdint.hpp>


int main(int argc, char* argv[]) {

  std::cout << "Running TestFWCoreParameterSetDescription from parameterSetDescription_t.cc" << std::endl;

  {
    art::ParameterSetDescription psetDesc;
    assert(!psetDesc.anythingAllowed());
    assert(!psetDesc.isUnknown());
    assert(psetDesc.parameter_begin() == psetDesc.parameter_end());

    art::ParameterSet params;
    psetDesc.validate(params);

    params.addParameter<std::string>("testname",
                                   std::string("testvalue"));

    // Expect this to throw, parameter not in description
    try {
      psetDesc.validate(params);
      assert(0);
    }
    catch(art::Exception) {
      // OK
    }

    psetDesc.setAllowAnything();
    assert(psetDesc.anythingAllowed());

    psetDesc.validate(params);
  }

  {
    art::ParameterSetDescription psetDesc;

    art::ParameterSet params;
    params.addParameter<std::string>("testname",
                                   std::string("testvalue"));
    psetDesc.setUnknown();
    assert(psetDesc.isUnknown());

    psetDesc.validate(params);
  }

  {
    // Test this type separately because I do not know how to
    // add an entry into a ParameterSet without FileInPath pointing
    // at a real file.
    art::ParameterSetDescription psetDesc;
    art::ParameterDescription * par = psetDesc.add<art::FileInPath>("fileInPath", art::FileInPath());
    assert(par->type() == art::k_FileInPath);
    assert(art::parameterTypeEnumToString(par->type()) == std::string("FileInPath"));
  }

  art::ParameterSetDescription psetDesc;
  art::ParameterSet pset;

  psetDesc.reserve(2);

  int a = 1;
  art::ParameterDescription * par = psetDesc.add<int>(std::string("ivalue"), a);
  pset.addParameter<int>("ivalue", a);
  assert(par != 0);
  assert(par->label() == std::string("ivalue"));
  assert(par->type() == art::k_int32);
  assert(par->isTracked() == true);
  assert(par->isOptional() == false);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("int32"));

  art::ParameterSetDescription::parameter_const_iterator parIter = psetDesc.parameter_begin();
  assert(parIter->operator->() == par);


  unsigned b = 2;
  par = psetDesc.add<unsigned>("uvalue", b);
  pset.addParameter<unsigned>("uvalue", b);
  assert(par != 0);
  assert(par->label() == std::string("uvalue"));
  assert(par->type() == art::k_uint32);
  assert(par->isTracked() == true);
  assert(par->isOptional() == false);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("uint32"));

  parIter = psetDesc.parameter_begin();
  ++parIter;
  assert(parIter->operator->() == par);

  boost::int64_t c = 3;
  par = psetDesc.addUntracked<boost::int64_t>(std::string("i64value"), c);
  pset.addUntrackedParameter<boost::int64_t>("i64value", c);
  assert(par != 0);
  assert(par->label() == std::string("i64value"));
  assert(par->type() == art::k_int64);
  assert(par->isTracked() == false);
  assert(par->isOptional() == false);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("int64"));

  boost::uint64_t d = 4;
  par = psetDesc.addUntracked<boost::uint64_t>("u64value", d);
  pset.addUntrackedParameter<boost::uint64_t>("u64value", d);
  assert(par != 0);
  assert(par->label() == std::string("u64value"));
  assert(par->type() == art::k_uint64);
  assert(par->isTracked() == false);
  assert(par->isOptional() == false);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("uint64"));

  double e = 5;
  par = psetDesc.addOptional<double>(std::string("dvalue"), e);
  pset.addParameter<double>("dvalue", e);
  assert(par != 0);
  assert(par->label() == std::string("dvalue"));
  assert(par->type() == art::k_double);
  assert(par->isTracked() == true);
  assert(par->isOptional() == true);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("double"));

  bool f = true;
  par = psetDesc.addOptional<bool>("bvalue", f);
  pset.addParameter<bool>("bvalue", f);
  assert(par != 0);
  assert(par->label() == std::string("bvalue"));
  assert(par->type() == art::k_bool);
  assert(par->isTracked() == true);
  assert(par->isOptional() == true);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("bool"));

  std::string g;
  par = psetDesc.addOptionalUntracked<std::string>(std::string("svalue"), g);
  pset.addUntrackedParameter<std::string>("svalue", g);
  assert(par != 0);
  assert(par->label() == std::string("svalue"));
  assert(par->type() == art::k_string);
  assert(par->isTracked() == false);
  assert(par->isOptional() == true);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("string"));

  art::EventID h;
  par = psetDesc.addOptionalUntracked<art::EventID>("evalue", h);
  pset.addUntrackedParameter<art::EventID>("evalue", h);
  assert(par != 0);
  assert(par->label() == std::string("evalue"));
  assert(par->type() == art::k_EventID);
  assert(par->isTracked() == false);
  assert(par->isOptional() == true);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("EventID"));

  art::SubRunID i;
  par = psetDesc.add<art::SubRunID>("lvalue", i);
  pset.addParameter<art::SubRunID>("lvalue", i);
  assert(par->type() == art::k_SubRunID);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("SubRunID"));

  art::InputTag j;
  par = psetDesc.add<art::InputTag>("input", j);
  pset.addParameter<art::InputTag>("input", j);
  assert(par->type() == art::k_InputTag);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("InputTag"));

  std::vector<int> v1;
  par = psetDesc.add<std::vector<int> >("v1", v1);
  pset.addParameter<std::vector<int> >("v1", v1);
  assert(par->type() == art::k_vint32);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("vint32"));

  std::vector<unsigned> v2;
  par = psetDesc.add<std::vector<unsigned> >("v2", v2);
  pset.addParameter<std::vector<unsigned> >("v2", v2);
  assert(par->type() == art::k_vuint32);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("vuint32"));

  std::vector<boost::int64_t> v3;
  par = psetDesc.add<std::vector<boost::int64_t> >("v3", v3);
  pset.addParameter<std::vector<boost::int64_t> >("v3", v3);
  assert(par->type() == art::k_vint64);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("vint64"));

  std::vector<boost::uint64_t> v4;
  par = psetDesc.add<std::vector<boost::uint64_t> >("v4", v4);
  pset.addParameter<std::vector<boost::uint64_t> >("v4", v4);
  assert(par->type() == art::k_vuint64);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("vuint64"));

  std::vector<double> v5;
  par = psetDesc.add<std::vector<double> >("v5", v5);
  pset.addParameter<std::vector<double> >("v5", v5);
  assert(par->type() == art::k_vdouble);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("vdouble"));

  std::vector<std::string> v6;
  par = psetDesc.add<std::vector<std::string> >("v6", v6);
  pset.addParameter<std::vector<std::string> >("v6", v6);
  assert(par->type() == art::k_vstring);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("vstring"));

  std::vector<art::EventID> v7;
  par = psetDesc.add<std::vector<art::EventID> >("v7", v7);
  pset.addParameter<std::vector<art::EventID> >("v7", v7);
  assert(par->type() == art::k_VEventID);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("VEventID"));

  std::vector<art::SubRunID> v8;
  par = psetDesc.add<std::vector<art::SubRunID> >("v8", v8);
  pset.addParameter<std::vector<art::SubRunID> >("v8", v8);
  assert(par->type() == art::k_VSubRunID);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("VSubRunID"));

  std::vector<art::InputTag> v9;
  par = psetDesc.add<std::vector<art::InputTag> >("v9", v9);
  pset.addParameter<std::vector<art::InputTag> >("v9", v9);
  assert(par->type() == art::k_VInputTag);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("VInputTag"));

  art::ParameterSetDescription m;
  par = psetDesc.add<art::ParameterSetDescription>("psetDesc", m);
  art::ParameterSet p1;
  pset.addParameter<art::ParameterSet>("psetDesc", p1);
  assert(par->type() == art::k_PSet);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("PSet"));

  std::vector<art::ParameterSetDescription> v10;
  par = psetDesc.add<std::vector<art::ParameterSetDescription> >("psetDescVector", v10);
  std::vector<art::ParameterSet> vp1;
  pset.addParameter<std::vector<art::ParameterSet> >("psetDescVector", vp1);
  assert(par->type() == art::k_VPSet);
  assert(art::parameterTypeEnumToString(par->type()) == std::string("VPSet"));

  psetDesc.validate(pset);

  // Add a ParameterSetDescription nested in a ParameterSetDescription nested in
  // a vector in the top level ParameterSetDescription to see if the nesting is
  // working properly.

  art::ParameterSet nest2;
  nest2.addParameter<int>("intLevel2a", 1);
  nest2.addUntrackedParameter<int>("intLevel2b", 1);
  nest2.addParameter<int>("intLevel2e", 1);
  nest2.addUntrackedParameter<int>("intLevel2f", 1);

  art::ParameterSet nest1;
  nest1.addParameter<int>("intLevel1a", 1);
  nest1.addParameter<art::ParameterSet>("nestLevel1b", nest2);

  std::vector<art::ParameterSet> vPset;
  vPset.push_back(art::ParameterSet());
  vPset.push_back(nest1);

  pset.addUntrackedParameter<std::vector<art::ParameterSet> >("nestLevel0", vPset);


  std::vector<art::ParameterSetDescription> testDescriptions;
  testDescriptions.push_back(psetDesc);
  testDescriptions.push_back(psetDesc);
  testDescriptions.push_back(psetDesc);

  for (int i = 0; i < 3; ++i) {

    art::ParameterSetDescription nestLevel2;

    // for the first test do not put a parameter in the description
    // so there will be an extra parameter in the ParameterSet and
    // validation should fail.
    if (i > 0) par = nestLevel2.add<int>("intLevel2a", 1);

    // for the next test validation should pass

    // For the last test add an extra required parameter in the
    // description that is not in the ParameterSet.
    if (i == 2) par = nestLevel2.add<int>("intLevel2extra", 1);

    par = nestLevel2.addUntracked<int>("intLevel2b", 1);
    par = nestLevel2.addOptional<int>("intLevel2c", 1);
    par = nestLevel2.addOptionalUntracked<int>("intLevel2d", 1);
    par = nestLevel2.addOptional<int>("intLevel2e", 1);
    par = nestLevel2.addOptionalUntracked<int>("intLevel2f", 1);

    art::ParameterSetDescription nestLevel1;
    par = nestLevel1.add<int>("intLevel1a", 1);
    par = nestLevel1.add<art::ParameterSetDescription>("nestLevel1b", nestLevel2);

    std::vector<art::ParameterSetDescription> vDescs;
    vDescs.push_back(art::ParameterSetDescription());
    vDescs.push_back(nestLevel1);

    testDescriptions[i].addUntracked<std::vector<art::ParameterSetDescription> >("nestLevel0", vDescs);
  }

  // Now run the validation and make sure we get the expected results
  try {
    testDescriptions[0].validate(pset);
    assert(0);
  }
  catch(art::Exception) {
    // There should be an exception
  }

  // This one should pass validation with no exception
  testDescriptions[1].validate(pset);

  try {
    testDescriptions[2].validate(pset);
    assert(0);
  }
  catch(art::Exception) {
    // There should be an exception
  }


  // One more iteration, this time the purpose is to
  // test the parameterSetDescription accessors.
  art::ParameterSetDescription nestLevel2;
  par = nestLevel2.add<int>("intLevel2a", 1);
  assert(par->parameterSetDescription() == 0);
  assert(par->parameterSetDescriptions() == 0);
  art::ParameterDescription const& constParRef = *par;
  assert(constParRef.parameterSetDescription() == 0);
  assert(constParRef.parameterSetDescriptions() == 0);

  par = nestLevel2.addUntracked<int>("intLevel2b", 1);
  par = nestLevel2.addOptional<int>("intLevel2c", 1);
  par = nestLevel2.addOptionalUntracked<int>("intLevel2d", 1);
  par = nestLevel2.addOptional<int>("intLevel2e", 1);
  par = nestLevel2.addOptionalUntracked<int>("intLevel2f", 1);
  nestLevel2.setAllowAnything();

  art::ParameterSetDescription nestLevel1;
  par = nestLevel1.add<int>("intLevel1a", 1);
  par = nestLevel1.add<art::ParameterSetDescription>("nestLevel1b", nestLevel2);
  assert(par->parameterSetDescription() != 0);
  assert(par->parameterSetDescriptions() == 0);
  art::ParameterDescription const& constParRef2 = *par;
  assert(constParRef2.parameterSetDescription() != 0);
  assert(constParRef2.parameterSetDescriptions() == 0);

  assert(par->parameterSetDescription()->anythingAllowed() == true);
  assert(constParRef2.parameterSetDescription()->anythingAllowed() == true);

  std::vector<art::ParameterSetDescription> vDescs;
  vDescs.push_back(art::ParameterSetDescription());
  vDescs.push_back(nestLevel1);

  par = psetDesc.addUntracked<std::vector<art::ParameterSetDescription> >("nestLevel0", vDescs);
  assert(par->parameterSetDescription() == 0);
  assert(par->parameterSetDescriptions() != 0);
  art::ParameterDescription const& constParRef3 = *par;
  assert(constParRef3.parameterSetDescription() == 0);
  assert(constParRef3.parameterSetDescriptions() != 0);

  std::vector<art::ParameterSetDescription> * vec = par->parameterSetDescriptions();
  assert(vec->size() == 2);

  std::vector<art::ParameterSetDescription> const* vec2 = constParRef3.parameterSetDescriptions();
  assert(vec2->size() == 2);

  psetDesc.validate(pset);

  return 0;
}
