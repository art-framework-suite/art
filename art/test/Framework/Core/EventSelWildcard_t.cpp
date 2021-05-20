#include "art/Framework/Core/EventSelector.h"
#include "art/Persistency/Provenance/PathSpec.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <bitset>
#include <iostream>
#include <string>
#include <vector>

using namespace art;
using namespace fhicl;
using namespace std;
using art::PathSpec;

constexpr size_t numBits = 12; // There must be a better way than this but I
                               // choose to avoid modifying a whole slew of code
                               // using the array instead of push_back()s.
constexpr size_t numPatterns = 12;
constexpr size_t numTestMasks = 9;

typedef std::vector<std::vector<bool>> Answers;

typedef std::vector<string> Strings;
typedef std::vector<Strings> VStrings;
typedef std::vector<std::bitset<numBits>> VBools;

std::ostream&
operator<<(std::ostream& ost, const Strings& strings)
{
  for (auto const& element : strings) {
    ost << element << " ";
  }
  return ost;
}

void
testone(const Strings& paths,
        const Strings& pattern,
        const std::bitset<numBits>& mask,
        bool answer,
        int jmask)
{
  EventSelector selector{pattern};

  int number_of_trigger_paths = 0;
  std::vector<unsigned char> bitArray;

  HLTGlobalStatus bm(mask.size());
  HLTPathStatus const pass{art::hlt::Pass};
  HLTPathStatus const fail{art::hlt::Fail};
  HLTPathStatus const ex{art::hlt::Exception};
  HLTPathStatus const ready{art::hlt::Ready};

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

  TriggerResults const results_id{bm, trigger_pset.id()};
  bool const result = selector.acceptEvent(ScheduleID::first(), results_id);
  if (result != answer) {
    std::cerr << "failed to compare pattern with mask using pset ID: "
              << "correct=" << answer << " "
              << "results=" << std::boolalpha << result << '\n'
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
  Strings const paths{"HLTx1",
                      "HLTx2",
                      "HLTy1",
                      "HLTy2",
                      "CALIBx1",
                      "CALIBx2",
                      "CALIBy1",
                      "CALIBy2",
                      "DEBUGx1",
                      "DEBUGx2",
                      "DEBUGy1",
                      "DEBUGy2"};
  assert(size(paths) == numBits);

  // Create our test patterns.  Each of these will be tested against each mask.

  VStrings patterns(numPatterns);
  patterns[0] = {"*"};
  patterns[1] = {"!*"};
  patterns[2] = {"HLTx1", "HLTy1"};
  patterns[3] = {"CALIBx2", "!HLTx2"};
  patterns[4] = {"HLT*"};
  patterns[5] = {"!HLT*"};
  patterns[6] = {"DEBUG*1", "HLT?2"};
  patterns[7] = {"D*x1", "CALIBx*"};
  patterns[8] = {"HL*1", "C?LIB*2"};
  patterns[9] = {"H*x1"};
  patterns[10] = {"!H*x1"};
  patterns[11] = {"C?LIB*2"};

  auto to_bits = [](std::string bit_string) {
    // The provided string assumes that the first character corresponds to
    // index 0 of the bitset.  However, this is the opposite convention that
    // std::bitset uses.  We therefore reverse the characters.
    std::reverse(begin(bit_string), end(bit_string));
    return std::bitset<numBits>{bit_string};
  };

  VBools testmasks(numTestMasks);
  testmasks[0] = to_bits("000000000000");
  testmasks[1] = to_bits("111111111111");
  testmasks[2] = to_bits("100000000000");
  testmasks[3] = to_bits("010000000000");
  testmasks[4] = to_bits("000000001000");
  testmasks[5] = to_bits("111100100000");
  testmasks[6] = to_bits("000001000010");
  testmasks[7] = to_bits("101001100101");
  testmasks[8] =
    to_bits("000001001111"); // For j=8 only, the first HLTx1 (false)
                             // is reset to ready and the fifth
                             // CALIBx2 (true) is reset to exception.
  // Create the answers

  Answers ans;

  std::vector<bool> ansstar; // Answers for criteria star: {{ "*" }};
  ansstar.push_back(false);  // f f f f f f f f f f f f
  ansstar.push_back(true);   // t t t t t t t t t t t t
  ansstar.push_back(true);   // t f f f f f f f f f f f
  ansstar.push_back(true);   // f t f f f f f f f f f f
  ansstar.push_back(true);   // f f f f f f f f t f f f
  ansstar.push_back(true);   // t t t t f f t f f f f f
  ansstar.push_back(true);   // f f f f f t f f f f t f
  ansstar.push_back(true);   // t f f f f t t f f t f t
  ansstar.push_back(true);   // r f f f e t f f t t t t

  ans.push_back(ansstar);

  std::vector<bool> ansnotstar; // Answers for criteria notstar: {{ "!*" }};
  ansnotstar.push_back(true);   // f f f f f f f f f f f f
  ansnotstar.push_back(false);  // t t t t t t t t t t t t
  ansnotstar.push_back(false);  // t f f f f f f f f f f f
  ansnotstar.push_back(false);  // f t f f f f f f f f f f
  ansnotstar.push_back(false);  // f f f f f f f f t f f f
  ansnotstar.push_back(false);  // t t t t f f t f f f f f
  ansnotstar.push_back(false);  // f f f f f t f f f f t f
  ansnotstar.push_back(false);  // t f f f f t t f f t f t
  ansnotstar.push_back(false);  // r f f f e t f f t t t t

  ans.push_back(ansnotstar);

  std::vector<bool> ans0; // Answers for criteria 0:{{ "HLTx1", "HLTy1" }};
  ans0.push_back(false);  // f f f f f f f f f f f f
  ans0.push_back(true);   // t t t t t t t t t t t t
  ans0.push_back(true);   // t f f f f f f f f f f f
  ans0.push_back(false);  // f t f f f f f f f f f f
  ans0.push_back(false);  // f f f f f f f f t f f f
  ans0.push_back(true);   // t t t t f f t f f f f f
  ans0.push_back(false);  // f f f f f t f f f f t f
  ans0.push_back(true);   // t f f f f t t f f t f t
  ans0.push_back(false);  // r f f f e t f f t t t t

  ans.push_back(ans0);

  std::vector<bool> ans1; // Answers for criteria 1:{{"CALIBx2","!HLTx2"}};
  ans1.push_back(true);   // f f f f f f f f f f f f
  ans1.push_back(true);   // t t t t t t t t t t t t
  ans1.push_back(true);   // t f f f f f f f f f f f
  ans1.push_back(false);  // f t f f f f f f f f f f
  ans1.push_back(true);   // f f f f f f f f t f f f
  ans1.push_back(false);  // t t t t f f t f f f f f
  ans1.push_back(true);   // f f f f f t f f f f t f
  ans1.push_back(true);   // t f f f f t t f f t f t
  ans1.push_back(true);   // r f f f e t f f t t t t

  ans.push_back(ans1);

  std::vector<bool> ans2; // Answers for criteria 2:{{ "HLT*" }};
  ans2.push_back(false);  // f f f f f f f f f f f f
  ans2.push_back(true);   // t t t t t t t t t t t t
  ans2.push_back(true);   // t f f f f f f f f f f f
  ans2.push_back(true);   // f t f f f f f f f f f f
  ans2.push_back(false);  // f f f f f f f f t f f f
  ans2.push_back(true);   // t t t t f f t f f f f f
  ans2.push_back(false);  // f f f f f t f f f f t f
  ans2.push_back(true);   // t f f f f t t f f t f t
  ans2.push_back(false);  // r f f f e t f f t t t t

  ans.push_back(ans2);

  std::vector<bool> ans3; // Answers for criteria 3:{{ "!HLT*" }};
  ans3.push_back(true);   // f f f f f f f f f f f f
  ans3.push_back(false);  // t t t t t t t t t t t t
  ans3.push_back(false);  // t f f f f f f f f f f f
  ans3.push_back(false);  // f t f f f f f f f f f f
  ans3.push_back(true);   // f f f f f f f f t f f f
  ans3.push_back(false);  // t t t t f f t f f f f f
  ans3.push_back(true);   // f f f f f t f f f f t f
  ans3.push_back(false);  // t f f f f t t f f t f t
  ans3.push_back(false);  // r f f f e t f f t t t t // ready is not fail

  ans.push_back(ans3);

  std::vector<bool> ans4; // Answers for criteria 4:{{"DEBUG*1","HLT?2"}};;
  ans4.push_back(false);  // f f f f f f f f f f f f
  ans4.push_back(true);   // t t t t t t t t t t t t
  ans4.push_back(false);  // t f f f f f f f f f f f
  ans4.push_back(true);   // f t f f f f f f f f f f
  ans4.push_back(true);   // f f f f f f f f t f f f
  ans4.push_back(true);   // t t t t f f t f f f f f
  ans4.push_back(true);   // f f f f f t f f f f t f
  ans4.push_back(false);  // t f f f f t t f f t f t
  ans4.push_back(true);   // r f f f e t f f t t t t

  ans.push_back(ans4);

  std::vector<bool> ans5; // Answers for criteria 5:{{ "D*x1", "CALIBx*" }};
  ans5.push_back(false);  // f f f f f f f f f f f f
  ans5.push_back(true);   // t t t t t t t t t t t t
  ans5.push_back(false);  // t f f f f f f f f f f f
  ans5.push_back(false);  // f t f f f f f f f f f f
  ans5.push_back(true);   // f f f f f f f f t f f f
  ans5.push_back(false);  // t t t t f f t f f f f f
  ans5.push_back(true);   // f f f f f t f f f f t f
  ans5.push_back(true);   // t f f f f t t f f t f t
  ans5.push_back(true);   // r f f f e t f f t t t t

  ans.push_back(ans5);

  std::vector<bool> ans6; // Answers for criteria 6:{{ "HL*1", "C?LIB*2" }};
  ans6.push_back(false);  // f f f f f f f f f f f f
  ans6.push_back(true);   // t t t t t t t t t t t t
  ans6.push_back(true);   // t f f f f f f f f f f f
  ans6.push_back(false);  // f t f f f f f f f f f f
  ans6.push_back(false);  // f f f f f f f f t f f f
  ans6.push_back(true);   // t t t t f f t f f f f f
  ans6.push_back(true);   // f f f f f t f f f f t f
  ans6.push_back(true);   // t f f f f t t f f t f t
  ans6.push_back(true);   // r f f f e t f f t t t t

  ans.push_back(ans6);

  std::vector<bool> ans7; // Answers for criteria7:{{ "H*x1" }};
  ans7.push_back(false);  // f f f f f f f f f f f f
  ans7.push_back(true);   // t t t t t t t t t t t t
  ans7.push_back(true);   // t f f f f f f f f f f f
  ans7.push_back(false);  // f t f f f f f f f f f f
  ans7.push_back(false);  // f f f f f f f f t f f f
  ans7.push_back(true);   // t t t t f f t f f f f f
  ans7.push_back(false);  // f f f f f t f f f f t f
  ans7.push_back(true);   // t f f f f t t f f t f t
  ans7.push_back(false);  // r f f f e t f f t t t t

  ans.push_back(ans7);

  std::vector<bool> ans8; // Answers for criteria8:{{ "!H*x1" }};
  ans8.push_back(true);   // f f f f f f f f f f f f
  ans8.push_back(false);  // t t t t t t t t t t t t
  ans8.push_back(false);  // t f f f f f f f f f f f
  ans8.push_back(true);   // f t f f f f f f f f f f
  ans8.push_back(true);   // f f f f f f f f t f f f
  ans8.push_back(false);  // t t t t f f t f f f f f
  ans8.push_back(true);   // f f f f f t f f f f t f
  ans8.push_back(false);  // t f f f f t t f f t f t
  ans8.push_back(
    false); // r f f f e t f f t t t t -- false because ready does not
            //                            itself cause an accept

  ans.push_back(ans8);

  std::vector<bool> ans9; // Answers for criteria 9:{{ "C?LIB*2" }};
  ans9.push_back(false);  // f f f f f f f f f f f f
  ans9.push_back(true);   // t t t t t t t t t t t t
  ans9.push_back(false);  // t f f f f f f f f f f f
  ans9.push_back(false);  // f t f f f f f f f f f f
  ans9.push_back(false);  // f f f f f f f f t f f f
  ans9.push_back(false);  // t t t t f f t f f f f f
  ans9.push_back(true);   // f f f f f t f f f f t f
  ans9.push_back(true);   // t f f f f t t f f t f t
  ans9.push_back(true);   // r f f f e t f f t t t t

  ans.push_back(ans9);

  Strings bit_qualified_paths;
  size_t i = 0;
  for (auto const& name : paths) {
    bit_qualified_paths.push_back(std::to_string(i) + ':' + name);
  }

  // We are ready to run some tests

  testall(bit_qualified_paths, patterns, testmasks, ans);
  return 0;
}
