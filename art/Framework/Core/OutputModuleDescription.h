#ifndef art_Framework_Core_OutputModuleDescription_h
#define art_Framework_Core_OutputModuleDescription_h

/*----------------------------------------------------------------------

OutputModuleDescription : the stuff that is needed to configure an
output module that does not come in through the ParameterSet


----------------------------------------------------------------------*/
namespace art {

  struct OutputModuleDescription {
    OutputModuleDescription() : maxEvents_(-1) {}
    OutputModuleDescription(int maxEvents) :
      maxEvents_(maxEvents)
    {}
    int maxEvents_;
  };
}

#endif /* art_Framework_Core_OutputModuleDescription_h */

// Local Variables:
// mode: c++
// End:
