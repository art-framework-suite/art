#include "art/Framework/Core/Worker.h"

using mf::LogError;


namespace art {
  namespace {

    class ModuleBeginJobSignalSentry {
public:
      ModuleBeginJobSignalSentry(ActivityRegistry* a, ModuleDescription& md):a_(a), md_(&md) {
        if(a_) a_->preModuleBeginJobSignal_(*md_);
      }
      ~ModuleBeginJobSignalSentry() {
        if(a_) a_->postModuleBeginJobSignal_(*md_);
      }
private:
      ActivityRegistry* a_;
      ModuleDescription* md_;
    };

    class ModuleEndJobSignalSentry {
public:
      ModuleEndJobSignalSentry(ActivityRegistry* a, ModuleDescription& md):a_(a), md_(&md) {
        if(a_) a_->preModuleEndJobSignal_(*md_);
      }
      ~ModuleEndJobSignalSentry() {
        if(a_) a_->postModuleEndJobSignal_(*md_);
      }
private:
      ActivityRegistry* a_;
      ModuleDescription* md_;
    };

  }  // namespace

  Worker::Worker(ModuleDescription const& iMD,
                 WorkerParams const& iWP) :
    stopwatch_(new RunStopwatch::StopwatchPointer::element_type),
    timesRun_(),
    timesVisited_(),
    timesPassed_(),
    timesFailed_(),
    timesExcept_(),
    state_(Ready),
    md_(iMD),
    actions_(iWP.actions_),
    cached_exception_(),
    actReg_()
  {
  }

  Worker::~Worker() {
  }

  void Worker::setActivityRegistry(boost::shared_ptr<ActivityRegistry> areg) {
    actReg_ = areg;
  }

  void Worker::beginJob() {

    try {
        ModuleBeginJobSignalSentry cpp(actReg_.get(), md_);
        implBeginJob();
    }
    catch(cet::exception& e) {
        // should event id be included?
        LogError("BeginJob")
          << "A cet::exception is going through " << workerType() << ":\n";

        e << "A cet::exception is going through " << workerType() << ":\n"
          << description();
        throw art::Exception(errors::OtherCMS, std::string(), e);
    }
    catch(std::bad_alloc& e) {
        LogError("BeginJob")
          << "A std::bad_alloc is going through " << workerType() << ":\n"
          << description() << "\n";
        throw;
    }
    catch(std::exception& e) {
        LogError("BeginJob")
          << "A std::exception is going through " << workerType() << ":\n"
          << description() << "\n";
        throw art::Exception(errors::StdException)
          << "A std::exception is going through " << workerType() << ":\n"
          << description() << "\n";
    }
    catch(std::string& s) {
        LogError("BeginJob")
          << "module caught an std::string during beginJob\n";

        throw art::Exception(errors::BadExceptionType)
          << "std::string = " << s << "\n"
          << description() << "\n";
    }
    catch(char const* c) {
        LogError("BeginJob")
          << "module caught an const char* during beginJob\n";

        throw art::Exception(errors::BadExceptionType)
          << "cstring = " << c << "\n"
          << description();
    }
    catch(...) {
        LogError("BeginJob")
          << "An unknown Exception occurred in\n" << description() << "\n";
        throw art::Exception(errors::Unknown)
          << "An unknown Exception occurred in\n" << description() << "\n";
    }
  }

  void Worker::endJob() {
    try {
        ModuleEndJobSignalSentry cpp(actReg_.get(), md_);
        implEndJob();
    }
    catch(cet::exception& e) {
        LogError("EndJob")
          << "A cet::exception is going through " << workerType() << ":\n";

        // should event id be included?
        e << "A cet::exception is going through " << workerType() << ":\n"
          << description();
        throw art::Exception(errors::OtherCMS, std::string(), e);
    }
    catch(std::bad_alloc& e) {
        LogError("EndJob")
          << "A std::bad_alloc is going through " << workerType() << ":\n"
          << description() << "\n";
        throw;
    }
    catch(std::exception& e) {
        LogError("EndJob")
          << "An std::exception is going through " << workerType() << ":\n"
          << description() << "\n";
        throw art::Exception(errors::StdException)
          << "A std::exception is going through " << workerType() << ":\n"
          << description() << "\n";
    }
    catch(std::string& s) {
        LogError("EndJob")
          << "module caught an std::string during endJob\n";

        throw art::Exception(errors::BadExceptionType)
          << "std::string = " << s << "\n"
          << description() << "\n";
    }
    catch(char const* c) {
        LogError("EndJob")
          << "module caught an const char* during endJob\n";

        throw art::Exception(errors::BadExceptionType)
          << "cstring = " << c << "\n"
          << description() << "\n";
    }
    catch(...) {
        LogError("EndJob")
          << "An unknown Exception occurred in\n" << description() << "\n";
        throw art::Exception(errors::Unknown)
          << "An unknown Exception occurred in\n" << description() << "\n";
    }
  }

}  // art
