#include "art/Framework/Services/Optional/SimpleInteraction.h"
#include "cetlib/container_algorithms.h"

#include <cerrno>
#include <cstdlib>
#include <iostream>

using namespace art;
using namespace std;

namespace ui {

  SimpleInteraction::SimpleInteraction(SimpleInteraction::Parameters const&,
                                       ActivityRegistry& iReg):
    UserInteraction{iReg}
  {
  }

  void SimpleInteraction::moduleList(std::vector<ModuleInfo> const& mi)
  {
    infos_ = mi;
  }

  void SimpleInteraction::pickModule()
  {
    bool done {false};
    while (!done) {
      string entered;
      int which {};
      cout << "You really want to reconfigure one of the modules, do you not?" << endl;
      cet::for_all_with_index(infos_,
                              [](unsigned const i, auto const& mi) {
                                cout << i << ". " << mi.label << " " << mi.class_name << "\n";
                              });
      cout << "which one do you want (e for end)? ";
      cout.flush();
      cin >> entered;
      // just continue if nothing entered
      if (entered.empty()) { done = true; continue; }
      if (entered[0] == 'e') { done = true; continue; }
      which = strtol(entered.c_str(), 0, 10);
      if (which == 0 && errno == EINVAL) { done = true; continue; }
      if (which >= 0 && size_t(which) < infos_.size())
      { callReconfigure(which, infos_[which].pset); }
      else
      { cerr << "Not valid module number. Try again.\n"; }
    }
  }

  UserInteraction::NextStep SimpleInteraction::nextAction()
  {
    int which {3};
    while (true) {
      cout << " finished event:" << std::endl;
      cout << "what do you want to do?" << endl;
      cout << " 0. continue\n"
           << " 1. reprocess current event\n"
           << " 2. restart at the first event\n" << endl;
      cout << "? ";
      cout.flush();
      cin >> which;
      if (which < 0 || which > Invalid) {
        cout << "invalid entry, try again." << endl;
      }
      else
      { break; }
    }
    return static_cast<NextStep>(which);
  }
}

DEFINE_ART_SERVICE(ui::SimpleInteraction)
