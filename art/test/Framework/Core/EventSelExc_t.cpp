// This tests:
//   Behavior of EventSelector when some trigger bits may be Exception or Ready
//    01 - Non-wildcard positives, exception accepted
//    02 - Non-wildcard negatives, exception accepted
//    03 - Non-wildcard positives and negatives mixed, exception accepted
//    04 - Non-wildcard positives, exception not accepted
//    05 - Non-wildcard negatives, exception not accepted, mixed with 01 case
//    06 - Wildcard positives, exception accepted
//    07 - Wildcard negatives, exception accepted
//    08 - Wildcard positives, exception not accepted
//    09 - Wildcard negatives, exception not accepted
//    10 - Everything except exceptions
//    11 - Exception demanded
//    12 - Specific and wildcarded exceptions
//    13 - Everything - also tests that it accepts all Ready

#include "art/Framework/Core/EventSelector.h"
#include "art/Persistency/Provenance/PathSpec.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

using namespace art;
using namespace fhicl;
using namespace std;
using namespace string_literals;
using art::PathSpec;

using Strings = std::vector<std::string>;
using PathSpecifiers = std::vector<std::string>;

// Name all our paths. We have as many paths as there are trigger
// bits.

Strings const trigger_path_names{"0:ap1",
                                 "1:ap2",
                                 "2:aq1",
                                 "3:aq2",
                                 "4:bp1",
                                 "5:bp2",
                                 "6:bq1",
                                 "7:bq2"};

HLTPathStatus const pass{art::hlt::Pass};
HLTPathStatus const fail{art::hlt::Fail};
HLTPathStatus const excp{art::hlt::Exception};
HLTPathStatus const redy{art::hlt::Ready};

struct TrigResults {
  std::vector<HLTPathStatus> bit;
  TrigResults(std::initializer_list<HLTPathStatus> status) : bit{status}
  {
    assert(size(bit) == size(trigger_path_names));
  }
};

std::ostream&
operator<<(std::ostream& ost, const Strings& strings)
{
  for (auto const& element : strings) {
    ost << element << " ";
  }
  return ost;
}

std::ostream&
operator<<(std::ostream& ost, const TrigResults& tr)
{
  for (unsigned int i = 0; i < tr.bit.size(); ++i) {
    HLTPathStatus b = tr.bit[i];
    if (b.state() == art::hlt::Ready)
      ost << "ready ";
    if (b.state() == art::hlt::Pass)
      ost << "pass  ";
    if (b.state() == art::hlt::Fail)
      ost << "fail  ";
    if (b.state() == art::hlt::Exception)
      ost << "excp  ";
  }
  return ost;
}

void
evSelTest(PathSpecifiers const& path_specifiers,
          TrigResults const& tr,
          bool ans)
{
  EventSelector selector{path_specifiers};

  int number_of_trigger_paths = 0;
  std::vector<unsigned char> bitArray;

  HLTGlobalStatus bm(tr.bit.size());
  for (unsigned int b = 0; b < tr.bit.size(); ++b) {
    bm.at(b) = (tr.bit[b]);
    // There is an alternate version of the function acceptEvent
    // that takes an array of characters as an argument instead
    // of a TriggerResults object.  These next few lines build
    // that array so we can test that also.
    if ((number_of_trigger_paths % 4) == 0)
      bitArray.push_back(0);
    int byteIndex = number_of_trigger_paths / 4;
    int subIndex = number_of_trigger_paths % 4;
    bitArray[byteIndex] |= (static_cast<unsigned char>(bm.state(b)))
                           << (subIndex * 2);
    ++number_of_trigger_paths;
  }

  ParameterSet trigger_pset;
  trigger_pset.put<Strings>("trigger_paths", trigger_path_names);
  ParameterSetRegistry::put(trigger_pset);

  TriggerResults const results_id{bm, trigger_pset.id()};
  bool const result = selector.acceptEvent(ScheduleID::first(), results_id);
  if (result != ans) {
    std::cerr
      << "failed to compare pathspecs with trigger results using pset ID: "
      << "correct=" << ans << " "
      << "results=" << std::boolalpha << result << '\n'
      << "pathspecs =" << path_specifiers << '\n'
      << "trigger results = " << tr << '\n';
    abort();
  }
}

int
main()
{
  // 01 - Non-wildcard positives, exception accepted
  PathSpecifiers const ps_a{"ap2"};
  TrigResults tr_01{fail, pass, fail, fail, fail, fail, excp, fail};
  evSelTest(ps_a, tr_01, true);
  tr_01 = {fail, pass, fail, fail, fail, fail, fail, fail};
  evSelTest(ps_a, tr_01, true);
  tr_01 = {pass, fail, pass, pass, fail, fail, fail, pass};
  evSelTest(ps_a, tr_01, false);
  tr_01 = {pass, excp, pass, pass, fail, fail, fail, pass};
  evSelTest(ps_a, tr_01, false);
  tr_01 = {pass, redy, pass, pass, fail, fail, fail, pass};
  evSelTest(ps_a, tr_01, false);

  // 02 - Non-wildcard negatives, exception accepted
  PathSpecifiers const ps_b{"!aq2"};
  TrigResults tr_02{pass, pass, pass, fail, pass, pass, excp, pass};
  evSelTest(ps_b, tr_02, true);
  tr_02 = {pass, pass, pass, fail, pass, pass, pass, pass};
  evSelTest(ps_b, tr_02, true);
  tr_02 = {pass, pass, pass, pass, pass, pass, pass, pass};
  evSelTest(ps_b, tr_01, false);
  tr_02 = {pass, pass, pass, excp, pass, pass, pass, pass};
  evSelTest(ps_b, tr_01, false);
  tr_02 = {pass, pass, pass, redy, pass, pass, pass, pass};
  evSelTest(ps_b, tr_01, false);

  // 03 - Non-wildcard positives and negatives mixed, exception accepted
  PathSpecifiers const ps_c{"bp1", "!aq1", "!bq2"};
  TrigResults tr_03{pass, pass, pass, pass, fail, pass, pass, pass};
  evSelTest(ps_c, tr_03, false);
  tr_03 = {excp, pass, pass, pass, pass, pass, pass, pass};
  evSelTest(ps_c, tr_03, true);
  tr_03 = {excp, pass, fail, pass, fail, pass, pass, pass};
  evSelTest(ps_c, tr_03, true);
  tr_03 = {excp, pass, pass, pass, fail, pass, pass, fail};
  evSelTest(ps_c, tr_03, true);
  tr_03 = {redy, pass, pass, pass, pass, pass, pass, fail};
  evSelTest(ps_c, tr_03, true);

  // 04 - Non-wildcard positives, exception not accepted
  PathSpecifiers const ps_d{"ap2&noexception"};
  TrigResults tr_04{fail, pass, fail, fail, fail, fail, excp, fail};
  evSelTest(ps_d, tr_04, false);
  tr_04 = {fail, pass, fail, fail, fail, fail, fail, fail};
  evSelTest(ps_d, tr_04, true);
  tr_04 = {pass, fail, pass, pass, fail, fail, fail, pass};
  evSelTest(ps_d, tr_04, false);
  tr_04 = {pass, excp, pass, pass, fail, fail, fail, pass};
  evSelTest(ps_d, tr_04, false);
  tr_04 = {pass, pass, pass, pass, fail, fail, redy, pass};
  evSelTest(ps_d, tr_04, true);

  // 05 - Non-wildcard negatives, exception not accepted, mixed with 01 case
  PathSpecifiers const ps_e{"bp1", "!aq1&noexception", "!bq2"};
  TrigResults tr_05{pass, pass, pass, pass, fail, pass, pass, pass};
  evSelTest(ps_e, tr_05, false);
  tr_05 = {excp, pass, pass, pass, pass, pass, pass, pass};
  evSelTest(ps_e, tr_05, true);
  tr_05 = {pass, pass, fail, pass, fail, pass, pass, pass};
  evSelTest(ps_e, tr_05, true);
  tr_05 = {pass, pass, fail, pass, fail, pass, excp, pass};
  evSelTest(ps_e, tr_05, false);

  // 06 - Wildcard positives, exception accepted
  PathSpecifiers const ps_f{"a*2", "?p2"};
  TrigResults tr_06{fail, pass, fail, fail, fail, fail, excp, fail};
  evSelTest(ps_f, tr_06, true);
  tr_06 = {fail, fail, fail, pass, fail, fail, fail, fail};
  evSelTest(ps_f, tr_06, true);
  tr_06 = {pass, fail, pass, fail, fail, pass, excp, excp};
  evSelTest(ps_f, tr_06, true);
  tr_06 = {pass, fail, pass, fail, pass, fail, pass, pass};
  evSelTest(ps_f, tr_06, false);

  // 07 - Wildcard negatives, exception accepted
  PathSpecifiers const ps_g{"!*2", "!ap?"};
  TrigResults tr_07{fail, pass, fail, fail, fail, fail, fail, fail};
  evSelTest(ps_g, tr_07, false);
  tr_07 = {pass, fail, pass, fail, excp, fail, pass, fail};
  evSelTest(ps_g, tr_07, true);
  tr_07 = {fail, fail, pass, pass, fail, pass, excp, excp};
  evSelTest(ps_g, tr_07, true);
  tr_07 = {pass, fail, fail, fail, fail, fail, fail, redy};
  evSelTest(ps_g, tr_07, false);

  // 08 - Wildcard positives, exception not accepted
  PathSpecifiers const ps_h{"a*2&noexception", "?p2"};
  TrigResults tr_08{fail, pass, fail, fail, fail, fail, excp, fail};
  evSelTest(ps_h, tr_08, true);
  tr_08 = {fail, fail, fail, pass, fail, fail, excp, fail};
  evSelTest(ps_h, tr_08, false);
  tr_08 = {pass, fail, pass, fail, fail, pass, excp, excp};
  evSelTest(ps_h, tr_08, true);
  tr_08 = {pass, fail, pass, fail, pass, fail, pass, pass};
  evSelTest(ps_h, tr_08, false);
  tr_08 = {excp, fail, pass, pass, pass, fail, pass, pass};
  evSelTest(ps_h, tr_08, false);

  // 09 - Wildcard negatives, exception not accepted
  PathSpecifiers const ps_i{"!*2&noexception"};
  TrigResults tr_09{fail, pass, fail, fail, fail, fail, fail, fail};
  evSelTest(ps_i, tr_09, false);
  tr_09 = {pass, fail, pass, fail, excp, fail, pass, fail};
  evSelTest(ps_i, tr_09, false);
  tr_09 = {fail, fail, pass, pass, fail, pass, excp, excp};
  evSelTest(ps_i, tr_09, false);
  tr_09 = {pass, fail, fail, fail, fail, fail, fail, redy};
  evSelTest(ps_i, tr_09, false);
  tr_09 = {fail, fail, pass, fail, fail, fail, pass, fail};
  evSelTest(ps_i, tr_09, true);

  // 10 - Everything except exceptions
  PathSpecifiers const ps_j{"*&noexception", "!*&noexception"};
  TrigResults tr_10{fail, pass, fail, fail, fail, fail, fail, fail};
  evSelTest(ps_j, tr_10, true);
  tr_10 = {pass, fail, pass, fail, excp, fail, pass, fail};
  evSelTest(ps_j, tr_10, false);
  tr_10 = {fail, fail, pass, pass, fail, pass, excp, excp};
  evSelTest(ps_j, tr_10, false);
  tr_10 = {fail, fail, fail, fail, fail, fail, fail, redy};
  evSelTest(ps_j, tr_10, false);
  tr_10 = {fail, fail, fail, fail, fail, fail, fail, fail};
  evSelTest(ps_j, tr_10, true);
  tr_10 = {pass, pass, pass, pass, pass, pass, pass, pass};
  evSelTest(ps_j, tr_10, true);
  tr_10 = {redy, redy, redy, redy, redy, redy, redy, redy};
  evSelTest(ps_j, tr_10, false); // rejected because all Ready fails !*

  // 11 - Exception demanded
  PathSpecifiers const ps_k{"exception@*"};
  TrigResults tr_11{fail, pass, fail, fail, fail, fail, fail, fail};
  evSelTest(ps_k, tr_11, false);
  tr_11 = {pass, fail, pass, fail, excp, fail, pass, fail};
  evSelTest(ps_k, tr_11, true);
  tr_11 = {redy, redy, redy, redy, redy, redy, redy, excp};
  evSelTest(ps_k, tr_11, true);
  tr_11 = {pass, pass, pass, pass, pass, pass, pass, excp};
  evSelTest(ps_k, tr_11, true);
  tr_11 = {redy, redy, redy, redy, redy, redy, redy, redy};
  evSelTest(ps_k, tr_11, false);
  tr_11 = {pass, fail, fail, fail, fail, fail, fail, excp};
  evSelTest(ps_k, tr_11, true);

  // 12 - Specific and wildcarded exceptions
  PathSpecifiers const ps_m{"exception@a*", "exception@bp1"};
  TrigResults tr_12{fail, pass, fail, fail, fail, fail, fail, fail};
  evSelTest(ps_m, tr_12, false);
  tr_12 = {pass, fail, pass, fail, excp, fail, pass, fail};
  evSelTest(ps_m, tr_12, true);
  tr_12 = {redy, redy, excp, redy, redy, redy, redy, excp};
  evSelTest(ps_m, tr_12, true);
  tr_12 = {pass, pass, pass, pass, pass, pass, pass, excp};
  evSelTest(ps_m, tr_12, false);

  // 13 - Everything - also tests that it accepts all Ready
  PathSpecifiers const ps_n{"*", "!*", "exception@*"};
  TrigResults tr_13{fail, pass, fail, fail, fail, fail, fail, fail};
  evSelTest(ps_n, tr_13, true);
  tr_13 = {pass, pass, pass, pass, pass, pass, pass, pass};
  evSelTest(ps_n, tr_13, true);
  tr_13 = {redy, redy, redy, redy, redy, redy, redy, excp};
  evSelTest(ps_n, tr_13, true);
  tr_13 = {redy, redy, redy, redy, redy, redy, redy, redy};
  evSelTest(ps_n, tr_13, true);
  tr_13 = {pass, pass, pass, pass, pass, pass, pass, excp};
  evSelTest(ps_n, tr_13, true);
  tr_13 = {excp, excp, excp, excp, excp, excp, excp, excp};
  evSelTest(ps_n, tr_13, true);
  tr_13 = {fail, redy, redy, redy, redy, redy, redy, redy};
  evSelTest(ps_n, tr_13, true);
  tr_13 = {fail, fail, fail, fail, fail, fail, fail, fail};
  evSelTest(ps_n, tr_13, true);
}
