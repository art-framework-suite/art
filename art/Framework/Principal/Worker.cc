#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"

using mf::LogError;

namespace {

  class ModuleSignalSentry {
  public:

    using PreSig_t  = art::GlobalSignal<art::detail::SignalResponseType::FIFO, void,
                                        art::ModuleDescription const &>;

    using PostSig_t = art::GlobalSignal<art::detail::SignalResponseType::LIFO, void,
                                        art::ModuleDescription const &>;

    ModuleSignalSentry( art::ActivityRegistry* a,
                        PreSig_t & pre,
                        PostSig_t & post,
                        art::ModuleDescription& md )
      : filled_(a)
      , post_(post)
      , md_  (&md)
    {
      if(filled_) pre.invoke(*md_);
    }
    ~ModuleSignalSentry() {
      if(filled_) post_.invoke(*md_);
    }
  private:
    bool filled_;
    PostSig_t post_;
    art::ModuleDescription* md_;
  };

}  // namespace

art::Worker::Worker(ModuleDescription const& iMD,
                    WorkerParams const& iWP)
  :
  timer_(),
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

art::Worker::~Worker() {
}

void
art::Worker::setActivityRegistry(cet::exempt_ptr<ActivityRegistry> areg) {
  actReg_ = std::move(areg);
}

void
art::Worker::beginJob() {
  try {
    ModuleSignalSentry cpp( actReg_.get(),
                            actReg_->sPreModuleBeginJob,
                            actReg_->sPostModuleBeginJob,
                            md_ );
    implBeginJob();
  }
  catch(cet::exception& e) {
    // should event id be included?
    LogError("BeginJob")
      << "A cet::exception is going through " << workerType() << ":\n";

    e << "A cet::exception is going through " << workerType() << ":\n"
      << description();
    throw art::Exception(errors::OtherArt, std::string(), e);
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

void
art::Worker::endJob() {
  try {
    ModuleSignalSentry cpp( actReg_.get(),
                            actReg_->sPreModuleEndJob,
                            actReg_->sPostModuleEndJob,
                            md_);
    implEndJob();
  }
  catch(cet::exception& e) {
    LogError("EndJob")
      << "A cet::exception is going through " << workerType() << ":\n";

    // should event id be included?
    e << "A cet::exception is going through " << workerType() << ":\n"
      << description();
    throw art::Exception(errors::OtherArt, std::string(), e);
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
      << description() << "\n" << e.what();
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

void
art::Worker::respondToOpenInputFile(FileBlock const& fb) {
  ModuleSignalSentry cpp(actReg_.get(),
                         actReg_->sPreModuleRespondToOpenInputFile,
                         actReg_->sPostModuleRespondToOpenInputFile,
                         md_);
  implRespondToOpenInputFile(fb);
}

void
art::Worker::respondToCloseInputFile(FileBlock const& fb) {
  ModuleSignalSentry cpp(actReg_.get(),
                         actReg_->sPreModuleRespondToCloseInputFile,
                         actReg_->sPostModuleRespondToCloseInputFile,
                         md_);
  implRespondToCloseInputFile(fb);
}

void
art::Worker::respondToOpenOutputFiles(FileBlock const& fb) {
  ModuleSignalSentry cpp(actReg_.get(),
                         actReg_->sPreModuleRespondToOpenOutputFiles,
                         actReg_->sPostModuleRespondToOpenOutputFiles,
                         md_);
  implRespondToOpenOutputFiles(fb);
}

void
art::Worker::respondToCloseOutputFiles(FileBlock const& fb) {
  ModuleSignalSentry cpp(actReg_.get(),
                         actReg_->sPreModuleRespondToCloseOutputFiles,
                         actReg_->sPostModuleRespondToCloseOutputFiles,
                         md_);
  implRespondToCloseOutputFiles(fb);
}
