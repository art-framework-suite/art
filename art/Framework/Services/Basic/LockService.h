#ifndef FWCore_Services_LockService_h
#define FWCore_Services_LockService_h

/*
  This class exists to only allow because ROOT is not thread-safe in its
  serialization/deserialization process at the moment.  It will hopefully
  be so very soon.  Here we have serialized access to some resource
  i.e. the one locking the mutex, where modules (using the module labels
  to identify them) are prevented from accessing that resource while running.

  There is only one mutex for the entire system.  We count one service
  object being constructed per thread that an event processor runs in.
  The current system only makes one event processor in one thread.
  If more than one event processor in more than one thread in the future,
  then I need to ensure that the service registry is not shared between
  any two threads running event processors.
 */

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "fhiclcpp/ParameterSet.h"

#include "boost/thread/mutex.hpp"

#include <string>
#include <vector>

namespace edm {
  namespace rootfix {

    class LockService
    {
    public:
      LockService(const fhicl::ParameterSet&,edm::ActivityRegistry&);
      ~LockService();

      boost::mutex& getLock() { return *lock_; }

      void postBeginJob();
      void postEndJob();

      void preSourceConstruction(const edm::ModuleDescription&);
      void postSourceConstruction(const edm::ModuleDescription&);

      void preEventProcessing(const edm::EventID&, const edm::Timestamp&);
      void postEventProcessing(const edm::Event&);

      void preSource();
      void postSource();


      void preModule(const edm::ModuleDescription&);
      void postModule(const edm::ModuleDescription&);

    private:
      boost::mutex* lock_;
      boost::mutex::scoped_lock* locker_; // what a hack!
      typedef std::vector<std::string> Labels;
      Labels labels_;
      bool lockSources_;
    };
  }
}

#endif  // FWCore_Services_LockService_h
