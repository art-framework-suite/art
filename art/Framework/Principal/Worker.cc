#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"

using mf::LogError;

art::Worker::Worker(ModuleDescription const& iMD, WorkerParams const& iWP)
  : md_{iMD}, actions_{iWP.actions_}
{}

void
art::Worker::setActivityRegistry(cet::exempt_ptr<ActivityRegistry> areg)
{
  actReg_ = std::move(areg);
}

void
art::Worker::beginJob() try {
  assert(actReg_.get() != nullptr);
  actReg_->sPreModuleBeginJob.invoke(md_);
  implBeginJob();
  actReg_->sPostModuleBeginJob.invoke(md_);
}
catch (cet::exception& e) {
  // should event id be included?
  LogError("BeginJob") << "A cet::exception is going through " << workerType()
                       << ":\n";

  e << "A cet::exception is going through " << workerType() << ":\n"
    << description();
  throw art::Exception(errors::OtherArt, std::string(), e);
}
catch (std::bad_alloc& e) {
  LogError("BeginJob") << "A std::bad_alloc is going through " << workerType()
                       << ":\n"
                       << description() << "\n";
  throw;
}
catch (std::exception& e) {
  LogError("BeginJob") << "A std::exception is going through " << workerType()
                       << ":\n"
                       << description() << "\n";
  throw art::Exception(errors::StdException)
    << "A std::exception is going through " << workerType() << ":\n"
    << description() << "\n";
}
catch (std::string& s) {
  LogError("BeginJob") << "module caught an std::string during beginJob\n";

  throw art::Exception(errors::BadExceptionType)
    << "std::string = " << s << "\n"
    << description() << "\n";
}
catch (char const* c) {
  LogError("BeginJob") << "module caught an const char* during beginJob\n";

  throw art::Exception(errors::BadExceptionType) << "cstring = " << c << "\n"
                                                 << description();
}
catch (...) {
  LogError("BeginJob") << "An unknown Exception occurred in\n"
                       << description() << "\n";
  throw art::Exception(errors::Unknown) << "An unknown Exception occurred in\n"
                                        << description() << "\n";
}

void
art::Worker::endJob() try {
  assert(actReg_.get() != nullptr);
  actReg_->sPreModuleEndJob.invoke(md_);
  implEndJob();
  actReg_->sPostModuleEndJob.invoke(md_);
}
catch (cet::exception& e) {
  LogError("EndJob") << "A cet::exception is going through " << workerType()
                     << ":\n";

  // should event id be included?
  e << "A cet::exception is going through " << workerType() << ":\n"
    << description();
  throw art::Exception(errors::OtherArt, std::string(), e);
}
catch (std::bad_alloc& e) {
  LogError("EndJob") << "A std::bad_alloc is going through " << workerType()
                     << ":\n"
                     << description() << "\n";
  throw;
}
catch (std::exception& e) {
  LogError("EndJob") << "An std::exception is going through " << workerType()
                     << ":\n"
                     << description() << "\n";
  throw art::Exception(errors::StdException)
    << "A std::exception is going through " << workerType() << ":\n"
    << description() << "\n"
    << e.what();
}
catch (std::string& s) {
  LogError("EndJob") << "module caught an std::string during endJob\n";

  throw art::Exception(errors::BadExceptionType)
    << "std::string = " << s << "\n"
    << description() << "\n";
}
catch (char const* c) {
  LogError("EndJob") << "module caught an const char* during endJob\n";

  throw art::Exception(errors::BadExceptionType) << "cstring = " << c << "\n"
                                                 << description() << "\n";
}
catch (...) {
  LogError("EndJob") << "An unknown Exception occurred in\n"
                     << description() << "\n";
  throw art::Exception(errors::Unknown) << "An unknown Exception occurred in\n"
                                        << description() << "\n";
}

void
art::Worker::respondToOpenInputFile(FileBlock const& fb)
{
  assert(actReg_.get() != nullptr);
  actReg_->sPreModuleRespondToOpenInputFile.invoke(md_);
  implRespondToOpenInputFile(fb);
  actReg_->sPostModuleRespondToOpenInputFile.invoke(md_);
}

void
art::Worker::respondToCloseInputFile(FileBlock const& fb)
{
  assert(actReg_.get() != nullptr);
  actReg_->sPreModuleRespondToCloseInputFile.invoke(md_);
  implRespondToCloseInputFile(fb);
  actReg_->sPostModuleRespondToCloseInputFile.invoke(md_);
}

void
art::Worker::respondToOpenOutputFiles(FileBlock const& fb)
{
  assert(actReg_.get() != nullptr);
  actReg_->sPreModuleRespondToOpenOutputFiles.invoke(md_);
  implRespondToOpenOutputFiles(fb);
  actReg_->sPostModuleRespondToOpenOutputFiles.invoke(md_);
}

void
art::Worker::respondToCloseOutputFiles(FileBlock const& fb)
{
  assert(actReg_.get() != nullptr);
  actReg_->sPreModuleRespondToCloseOutputFiles.invoke(md_);
  implRespondToCloseOutputFiles(fb);
  actReg_->sPostModuleRespondToCloseOutputFiles.invoke(md_);
}
