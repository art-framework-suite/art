#ifndef ART_FRAMEWORK_CORE_EVENTOBSERVER_H
#define ART_FRAMEWORK_CORE_EVENTOBSERVER_H
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

#endif
