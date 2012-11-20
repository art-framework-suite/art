#ifndef art_Framework_Services_UserInteraction_UserInteraction_h
#define art_Framework_Services_UserInteraction_UserInteraction_h

// -*- C++ -*-
//
// Package:     Services
// Class  :     UserInteraction
//
/*

 Description: Allows for reconfiguration of modules at
   start of event loop, and reprocessing, rewind, or continuation at the end.

*/
//
// Original Author:  Jim Kowalkowski and Marc Paterno
//         Created:  Wed Sep 27 2010
//

#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include <vector>
#include <string>

namespace art {
  class Worker;
  class InputSource;
}

namespace ui {

  class UserInteraction {
  public:
    typedef std::vector<art::Worker *> Workers;

    enum NextStep { NextEvent = 0, ReprocessEvent = 1, RewindFile = 2, Invalid = 3};

    struct ModuleInfo {
      ModuleInfo(std::string const & lab, std::string const & n,
                 fhicl::ParameterSet const & p):
        label(lab), class_name(n), pset(p) { }
      std::string label;
      std::string class_name;
      fhicl::ParameterSet pset;
    };

    explicit UserInteraction(art::ActivityRegistry &);
    virtual ~UserInteraction() = default;

    // must be supplied by user. called when the module list is
    // available.  this will supply user code with a list of
    // information about the configured modules.
    virtual void moduleList(std::vector<ModuleInfo> const &) = 0;

    // must be supplied by user, called when we want to tell the derived
    // service when to cause reconfigure by invoking callReconfigure().
    // called at the  beginning of each iteration of the event loop.
    virtual void pickModule() = 0;

    // must be supplied by user.  called when we need to know what
    // to do at the end of each iteration of the event loop.
    virtual NextStep nextAction() = 0;

    // the user code must call this to cause the reconfigure to be
    // executed in the module.  The argument is the index into the
    // worker (module) array.
    void callReconfigure(int module_index,
                         fhicl::ParameterSet const & pset);

  private:
    void preEvent(art::Event const & ev);
    void postEvent(art::Event const & ev);
    void postBeginJobWorkers(art::InputSource * is, std::vector<art::Worker *> const &);


    // void preModuleEvent(art::ModuleDescription const& md);
    // void postModuleEvent(art::ModuleDescription const& md);

    Workers workers_;
    art::InputSource * input_;
  };
}

#endif /* art_Framework_Services_UserInteraction_UserInteraction_h */

// Local Variables:
// mode: c++
// End:
