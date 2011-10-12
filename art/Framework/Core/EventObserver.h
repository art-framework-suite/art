#ifndef art_Framework_Core_EventObserver_h
#define art_Framework_Core_EventObserver_h
// Common base class for module which do not modify events, such as
// OutputModule and EDAnalyzer.

namespace art {
  class MasterProductRegistry;
  class ModuleDescription;

  class EventObserver;
}

class art::EventObserver {
public:
  bool modifiesEvent() const { return false; }

  // FIXME: One could obviate the need for this trivial implementation
  // by putting some type logic in WorkerT.
  void registerProducts(MasterProductRegistry &, ModuleDescription const &) {}

protected:
  virtual ~EventObserver();

};

#endif /* art_Framework_Core_EventObserver_h */

// Local Variables:
// mode: c++
// End:
