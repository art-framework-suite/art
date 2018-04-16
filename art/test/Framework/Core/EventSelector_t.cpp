#include "art/Framework/Core/EventSelector.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace art;
using namespace fhicl;
using namespace std;

constexpr size_t numBits = 5;
constexpr int numPatterns = 11;
constexpr int numMasks = 9;

typedef bool Answers[numPatterns][numMasks];
typedef std::vector<std::string> Strings;
typedef std::vector<Strings> VStrings;
typedef std::vector<bool> Bools;
typedef std::vector<Bools> VBools;

std::ostream&
operator<<(std::ostream& ost, const Strings& s)
{
  for (auto const& str : s) {
    ost << str << " ";
  }
  return ost;
}

std::ostream&
operator<<(std::ostream& ost, const Bools& b)
{
  for (unsigned int i = 0; i < b.size(); ++i) {
    ost << b[i] << " ";
  }
  return ost;
}

void
testone(const Strings& paths,
        const Strings& pattern,
        const Bools& mask,
        bool answer,
        int jmask)
{
  // There are 2 different ways to build the EventSelector.  Both
  // should give the same result.  We exercise both here.
  EventSelector select1(pattern, paths);
  EventSelector select2(pattern);

  int number_of_trigger_paths = 0;
  std::vector<unsigned char> bitArray;

  HLTGlobalStatus bm(mask.size());
  const HLTPathStatus pass = HLTPathStatus(art::hlt::Pass);
  const HLTPathStatus fail = HLTPathStatus(art::hlt::Fail);
  const HLTPathStatus ex = HLTPathStatus(art::hlt::Exception);
  const HLTPathStatus ready = HLTPathStatus(art::hlt::Ready);
  for (unsigned int b = 0; b < mask.size(); ++b) {
    bm[b] = (mask[b] ? pass : fail);

    // There is an alternate version of the function acceptEvent
    // that takes an array of characters as an argument instead
    // of a TriggerResults object.  These next few lines build
    // that array so we can test that also.
    if ((number_of_trigger_paths % 4) == 0)
      bitArray.push_back(0);
    int byteIndex = number_of_trigger_paths / 4;
    int subIndex = number_of_trigger_paths % 4;
    bitArray[byteIndex] |= (mask[b] ? art::hlt::Pass : art::hlt::Fail)
                           << (subIndex * 2);
    ++number_of_trigger_paths;
  }

  if (jmask == 8 && mask.size() > 4) {
    bm[0] = ready;
    bm[4] = ex;
    bitArray[0] = (bitArray[0] & 0xfc) | art::hlt::Ready;
    bitArray[1] = (bitArray[1] & 0xfc) | art::hlt::Exception;
  }

  ParameterSet trigger_pset;
  trigger_pset.put<Strings>("trigger_paths", paths);
  ParameterSetRegistry::put(trigger_pset);

  TriggerResults results_id(bm, trigger_pset.id());

  bool const a12 = select1.acceptEvent(results_id);
  bool const a13 = select2.acceptEvent(results_id);
  bool const a14 = select2.acceptEvent(results_id);

  if (a12 != answer || a13 != answer || a14 != answer) {
    std::cerr << "failed to compare pattern with mask using pset ID: "
              << "correct=" << answer << " "
              << "results=" << a12 << "  " << a13 << "  " << a14 << "\n"
              << "pattern=" << pattern << "\n"
              << "mask=" << mask << "\n"
              << "jmask = " << jmask << "\n";
    abort();
  }
}

void
testall(const Strings& paths,
        const VStrings& patterns,
        const VBools& masks,
        const Answers& answers)
{
  for (unsigned int i = 0; i < patterns.size(); ++i) {
    for (unsigned int j = 0; j < masks.size(); ++j) {
      testone(paths, patterns[i], masks[j], answers[i][j], j);
    }
  }
}

int
main()
{

  // Name all our paths. We have as many paths as there are trigger
  // bits.
  std::array<char const*, numBits> cpaths = {{"a1", "a2", "a3", "a4", "a5"}};
  Strings paths(cpaths.begin(), cpaths.end());

  std::array<char const*, 2> cw1 = {{"a1", "a2"}};
  std::array<char const*, 2> cw2 = {{"!a1", "!a2"}};
  std::array<char const*, 2> cw3 = {{"a1", "!a2"}};
  std::array<char const*, 1> cw4 = {{"*"}};
  std::array<char const*, 1> cw5 = {{"!*"}};
  std::array<char const*, 2> cw6 = {{"*", "!*"}};
  std::array<char const*, 2> cw7 = {{"*", "!a2"}};
  std::array<char const*, 2> cw8 = {{"!*", "a2"}};
  std::array<char const*, 3> cw9 = {{"a1", "a2", "a5"}};
  std::array<char const*, 2> cwA = {{"a3", "a4"}};
  std::array<char const*, 1> cwB = {{"!a5"}};

  VStrings patterns(numPatterns);
  patterns[0].insert(patterns[0].end(), cw1.begin(), cw1.end());
  patterns[1].insert(patterns[1].end(), cw2.begin(), cw2.end());
  patterns[2].insert(patterns[2].end(), cw3.begin(), cw3.end());
  patterns[3].insert(patterns[3].end(), cw4.begin(), cw4.end());
  patterns[4].insert(patterns[4].end(), cw5.begin(), cw5.end());
  patterns[5].insert(patterns[5].end(), cw6.begin(), cw6.end());
  patterns[6].insert(patterns[6].end(), cw7.begin(), cw7.end());
  patterns[7].insert(patterns[7].end(), cw8.begin(), cw8.end());
  patterns[8].insert(patterns[8].end(), cw9.begin(), cw9.end());
  patterns[9].insert(patterns[9].end(), cwA.begin(), cwA.end());
  patterns[10].insert(patterns[10].end(), cwB.begin(), cwB.end());

  std::array<bool, numBits> t1 = {{true, false, true, false, true}};
  std::array<bool, numBits> t2 = {{false, true, true, false, true}};
  std::array<bool, numBits> t3 = {{true, true, true, false, true}};
  std::array<bool, numBits> t4 = {{false, false, true, false, true}};
  std::array<bool, numBits> t5 = {{false, false, false, false, false}};
  std::array<bool, numBits> t6 = {{true, true, true, true, true}};
  std::array<bool, numBits> t7 = {{true, true, true, true, false}};
  std::array<bool, numBits> t8 = {{false, false, false, false, true}};
  std::array<bool, numBits> t9 =
    {{false, false, false, false, false}}; // for t9 only, above the
                                           // first is reset to ready
                                           // last is reset to exception

  VBools testmasks(numMasks);
  testmasks[0].insert(testmasks[0].end(), t1.begin(), t1.end());
  testmasks[1].insert(testmasks[1].end(), t2.begin(), t2.end());
  testmasks[2].insert(testmasks[2].end(), t3.begin(), t3.end());
  testmasks[3].insert(testmasks[3].end(), t4.begin(), t4.end());
  testmasks[4].insert(testmasks[4].end(), t5.begin(), t5.end());
  testmasks[5].insert(testmasks[5].end(), t6.begin(), t6.end());
  testmasks[6].insert(testmasks[6].end(), t7.begin(), t7.end());
  testmasks[7].insert(testmasks[7].end(), t8.begin(), t8.end());
  testmasks[8].insert(testmasks[8].end(), t9.begin(), t9.end());

  Answers ans = {
    {true, true, true, false, false, true, true, false, false},
    {true, true, false, true, true, false, false, true, true},
    {true, false, true, true, true, true, true, true, true},
    {true, true, true, true, false, true, true, true, false}, // last column
                                                              // changed due to
                                                              // treatment of
                                                              // excp
    {false, false, false, false, true, false, false, false, false},
    {true, true, true, true, true, true, true, true, false}, // last column
                                                             // changed due to
                                                             // treatment of
                                                             // excp
    {true, true, true, true, true, true, true, true, true},
    {false, true, true, false, true, true, true, false, false},
    {true, true, true, true, false, true, true, true, false}, // last column
                                                              // changed due to
                                                              // treatment of
                                                              // excp
    {true, true, true, true, false, true, true, false, false},
    {false, false, false, false, true, false, true, false, false} // last column
                                                                  // changed due
                                                                  // to
                                                                  // treatment
                                                                  // of excp
  };

  // We want to create the TriggerNamesService because it is used in
  // the tests.  We do that here, but first we need to build a minimal
  // parameter set to pass to its constructor.  Then we build the
  // service and setup the service system.
  ParameterSet proc_pset;
  ParameterSet physics_pset;

  std::string processName("HLT");
  proc_pset.put("process_name", processName);

  ParameterSet trigPaths;
  trigPaths.put("trigger_paths", paths);
  proc_pset.put("trigger_paths", trigPaths);

  Strings endPaths;
  proc_pset.put("end_paths", endPaths);

  // We do not care what is in these parameters for the test, they
  // just need to exist.
  Strings dummy;
  for (size_t i = 0; i < numBits; ++i) {
    physics_pset.put(paths[i], dummy);
  }
  proc_pset.put("physics", physics_pset);

  // Now create and setup the service
  art::ActivityRegistry aReg;

  auto servicesManager_ = make_unique<ServicesManager>(ParameterSet{}, aReg);
  ServiceRegistry::instance().setManager(servicesManager_.get());

  servicesManager_->put(std::unique_ptr<art::TriggerNamesService>(new art::TriggerNamesService(paths, processName, false, trigPaths, physics_pset)));

  // We are ready to run some tests
  testall(paths, patterns, testmasks, ans);
  return 0;
}
