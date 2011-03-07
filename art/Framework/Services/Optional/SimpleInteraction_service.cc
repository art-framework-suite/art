// -*- C++ -*-
//
// Package:     Services
// Class  :     Guess
// 

// system include files
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include "art/Framework/Services/Optional/SimpleInteraction.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

using namespace art;
using namespace std;

namespace ui {

  SimpleInteraction::SimpleInteraction(fhicl::ParameterSet const& iPS,
				       ActivityRegistry& iReg):
    UserInteraction(iReg)
  {
  }

  void SimpleInteraction::moduleList(std::vector<ModuleInfo> const& mi)
  {
    infos_ = mi;
  }

  void SimpleInteraction::pickModule()
  {
    bool done=false;

    while(!done)
      {
	string entered;
	int which=0;

	cout << "You really want to reconfigure one of the modules, do you not?" << endl;
	std::vector<ModuleInfo>::iterator ib(infos_.begin()),
	  ie(infos_.end());

	for(which=0;ib!=ie;++ib,++which)
	  {
	    // std::cout << ib->pset << "\n\n";
	    std::cout << which << ". " << ib->label << " " 
		      << ib->class_name << "\n";
	  }

	cout << "which one do you want (e for end)? ";
	cout.flush();
	cin >> entered;

	// just continue if nothing entered
	if(entered.empty()) { done=true; continue; }
	if(entered[0]=='e') { done=true; continue; }

	which = strtol(entered.c_str(),0,10);

	if(which==0 && errno==EINVAL) { done=true; continue; }

	if (which >= 0 && which < infos_.size())
	  callReconfigure(which,infos_[which].pset);
	else
	  cerr << "bad, bad, bad.  Not valid module number. Try again.\n";
      }
  }
  
  UserInteraction::NextStep SimpleInteraction::nextAction()
  {
    int which=3;

    while(true)
      {
	std::cout <<" finished event:"<<std::endl;
	cout << "what do you want to do?" << endl;
	cout << " 0. continue\n";
	cout << " 1. reprocess current event\n";
	cout << " 2. restart at the first event\n" << endl;
	cout << "? ";
	cout.flush();
	cin >> which;

	if (which<0 || which>Invalid)
	  { 
	    cout << "invalid entry, try again." << endl;
	  }
	else
	  break;
      }

    return (NextStep)which;
  }  
}

DEFINE_ART_SERVICE(ui::SimpleInteraction);
