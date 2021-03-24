#include "art/Framework/Core/EventSelector.h"
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
    bm.at(b) = (mask[b] ? pass : fail);

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
    bm.at(0) = ready;
    bm.at(4) = ex;
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
  Strings const paths{"a1", "a2", "a3", "a4", "a5"};
  assert(size(paths) == numBits);

  VStrings patterns(numPatterns);
  patterns[0] = {"a1", "a2"};
  patterns[1] = {"!a1", "!a2"};
  patterns[2] = {"a1", "!a2"};
  patterns[3] = {"*"};
  patterns[4] = {"!*"};
  patterns[5] = {"*", "!*"};
  patterns[6] = {"*", "!a2"};
  patterns[7] = {"!*", "a2"};
  patterns[8] = {"a1", "a2", "a5"};
  patterns[9] = {"a3", "a4"};
  patterns[10] = {"!a5"};

  VBools testmasks(numMasks);
  testmasks[0] = {true, false, true, false, true};
  testmasks[1] = {false, true, true, false, true};
  testmasks[2] = {true, true, true, false, true};
  testmasks[3] = {false, false, true, false, true};
  testmasks[4] = {false, false, false, false, false};
  testmasks[5] = {true, true, true, true, true};
  testmasks[6] = {true, true, true, true, false};
  testmasks[7] = {false, false, false, false, true};
  testmasks[8] = {false,
                  false,
                  false,
                  false,
                  false}; // Not sure why this duplicates [4] above

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

  Strings bit_qualified_paths;
  size_t i = 0;
  for (auto const& name : paths) {
    bit_qualified_paths.push_back(to_string(i) + ':' + name);
  }

  // We are ready to run some tests
  testall(bit_qualified_paths, patterns, testmasks, ans);
  return 0;
}
