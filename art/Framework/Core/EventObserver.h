#ifndef art_Framework_Core_EventObserver_h
#define art_Framework_Core_EventObserver_h
// Common base class for module which do not modify events, such as
// OutputModule and EDAnalyzer.

namespace art {
   class EventObserver {
   public:
      bool modifiesEvent() const { return false; }

   protected:
      virtual ~EventObserver();

   };
}

#endif /* art_Framework_Core_EventObserver_h */

// Local Variables:
// mode: c++
// End:
