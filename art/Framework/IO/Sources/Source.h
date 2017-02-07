#ifndef art_Framework_IO_Sources_Source_h
#define art_Framework_IO_Sources_Source_h

// ======================================================================
//
// The Source class template is used to create InputSources which
// are capable of reading Runs, SubRuns and Events from non-standard
// input files. Sources instantiated from Source are *not* random
// access sources.
//
// The Source class template requires the use of a type T as its
// template parameter, satisfying the conditions outlined below. In
// one's XXX_module.cc class one must provide a typedef and module macro
// call along the lines of:
//
// namespace arttest {
//   typedef art::Source<GeneratorTestDetail> GeneratorTest;
// }
//
// DEFINE_ART_INPUT_SOURCE(arttest::GeneratorTest)
//
// However, there are several "flavors" of InputSource possible using
// this template, and one may wish to specify them using the "type
// traits" found in SourceTraits.h. Type traits are simple class
// templates that are used to signal properties of the classes used as
// their template arguments. Specialization is common. There are many
// examples of type traits in the standard, such as std::is_const<T> or
// std::is_integral<T>. Any traits you wish to specialize must be
// defined *after* the definition of your detail class T, but *before*
// the typedef above which will attempt to instantiate them. See
// SourceTraits.h for descriptions of the different traits one might
// wish to apply.
//
// The type T must supply the following non-static member functions:
//
//    * Construct an object of type T. The ParameterSet provided will be
//    that constructed by the 'source' statement in the job
//    configuration file. The ProductRegistryHelper must be used to
//    register products to be reconstituted by this source.
//
//      T(fhicl::ParameterSet const&,
//        art::ProductRegistryHelper&,
//        art::SourceHelper const&);
//
//    * Open the file of the given name, returning a new fileblock in
//    fb. If readFile is unable to return a valid FileBlock it should
//    throw. Suggestions for suitable exceptions are:
//    art::Exception(art::errors::FileOpenError) or
//    art::Exception(art::errors::FileReadError).
//
//      void readFile(std::string const& filename,
//                    art::FileBlock*& fb);
//
//    * Read the next part of the current file. Return false if nothing
//    was read; return true and set the appropriate 'out' arguments if
//    something was read.
//
//      bool readNext(art::RunPrincipal const* const inR,
//                    art::SubRunPrincipal const* const inSR,
//                    art::RunPrincipal*& outR,
//                    art::SubRunPrincipal*& outSR,
//                    art::EventPrincipal*& outE);
//
//    * After readNext has returned false, the behavior differs
//    depending on whether Source_Generator<XXX>::value is true or
//    false. If false (the default), then readFile(...) will be called
//    provided there is an unused string remaining in
//    source.fileNames. If true, then the source will finish unless
//    there exists an *optional* function:
//
//      bool hasMoreData(); // or
//
//      bool hasMoreData() const;
//
//    which returns true.
//
//    * Close the current input file.
//
//      void closeCurrentFile();
//
// ======================================================================

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/SourceHelper.h"
#include "art/Framework/IO/Sources/SourceTraits.h"
#include "art/Framework/IO/Sources/detail/FileNamesHandler.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "cetlib/detail/metaprogramming.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"

#include <algorithm>
#include <memory>
#include <type_traits>

// ----------------------------------------------------------------------

namespace art {
  template <class T>
  class Source;

  namespace detail {
    // Template metaprogramming.
    using cet::detail::enable_if_function_exists_t;

    template <typename T, typename = void>
    struct has_hasMoreData : std::false_type {};

    template <typename T>
    struct has_hasMoreData<T, enable_if_function_exists_t<bool(T::*)(), &T::hasMoreData>> : std::true_type {};

    template <typename T>
    struct has_hasMoreData<T, enable_if_function_exists_t<bool(T::*)() const, &T::hasMoreData>> : std::true_type {};

    template <typename T>
    struct do_call_hasMoreData {
      bool operator()(T& t)
      {
        return t.hasMoreData();
      }
    };

    template <typename T>
    struct do_not_call_hasMoreData {
      bool operator()(T&)
      {
        return false;
      }
    };
  }
}

// No-one gets to override this class.
template <class T>
class art::Source final : public art::InputSource {
public:
  Source(Source<T> const&) = delete;
  Source<T>& operator=(Source<T> const&) = delete;

  typedef T SourceDetail;

  Source(fhicl::ParameterSet const& p,
         InputSourceDescription& d);

  input::ItemType nextItemType() override;
  RunID run() const override;
  SubRunID subRun() const override;

  std::unique_ptr<FileBlock> readFile(MasterProductRegistry& mpr) override;
  void closeFile() override;

  std::unique_ptr<RunPrincipal> readRun() override;

  std::unique_ptr<SubRunPrincipal>
  readSubRun(cet::exempt_ptr<RunPrincipal> rp) override;

  std::unique_ptr<art::RangeSetHandler> runRangeSetHandler() override;
  std::unique_ptr<art::RangeSetHandler> subRunRangeSetHandler() override;

  using InputSource::readEvent;
  std::unique_ptr<EventPrincipal>
  readEvent(cet::exempt_ptr<SubRunPrincipal> srp) override;

private:

  cet::exempt_ptr<ActivityRegistry> act_;

  ProductRegistryHelper h_ {};
  SourceHelper sourceHelper_; // So it can be used by detail.
  SourceDetail detail_;
  input::ItemType state_ {input::IsInvalid};

  detail::FileNamesHandler<Source_wantFileServices<T>::value> fh_;
  std::string currentFileName_ {};

  std::unique_ptr<RunPrincipal> newRP_ {nullptr};
  std::unique_ptr<SubRunPrincipal> newSRP_ {nullptr};
  std::unique_ptr<EventPrincipal> newE_ {nullptr};

  // Cached Run and SubRun Principals used for users creating new
  // SubRun and Event Principals.  These are non owning!
  cet::exempt_ptr<RunPrincipal> cachedRP_ {nullptr};
  cet::exempt_ptr<SubRunPrincipal> cachedSRP_ {nullptr};

  bool pendingSubRun_ {false};
  bool pendingEvent_ {false};

  bool subRunIsNew_ {false};

  SubRunNumber_t remainingSubRuns_ {1};
  bool haveSRLimit_ {false};
  EventNumber_t remainingEvents_ {1};
  bool haveEventLimit_ {false};

  // Called in the constructor, to finish the process of product
  // registration.
  void finishProductRegistration_(InputSourceDescription& d);

  // Make detail_ try to read more stuff from its file. Cache any new
  // run/subrun/event. Throw an exception if we detect an error in the
  // data stream, or logic of the detail_ class. Move to the
  // appropriate new state.
  bool readNext_();

  // Check to see whether we have a new file to attempt to read,
  // moving to either the IsStop or IsFile state.
  void checkForNextFile_();

  // Call readNext_() and throw if we have not moved to the IsRun state.
  void readNextAndRequireRun_();

  // Call readNext_() and throw if we have moved to the IsEvent state.
  void readNextAndRefuseEvent_();

  // Test the newly read data for validity, given our current state.
  void throwIfInsane_(bool const result,
                      RunPrincipal* newR,
                      SubRunPrincipal* newSR,
                      EventPrincipal* newE) const;

  // Throw an art::Exception(errors::DataCorruption), with the given
  // message text.
  static void throwDataCorruption_(const char* msg);
};

template <class T>
art::Source<T>::Source(fhicl::ParameterSet const& p,
                       InputSourceDescription& d) :
  act_{&d.activityRegistry},
  sourceHelper_{d.moduleDescription},
  detail_{p, h_, sourceHelper_},
  fh_{p.get<std::vector<std::string>>("fileNames", std::vector<std::string>())}
{
  // Handle maxSubRuns parameter.
  int64_t maxSubRuns_par = p.get<int64_t>("maxSubRuns", -1);
  if (maxSubRuns_par > -1) {
    remainingSubRuns_ = maxSubRuns_par;
    haveSRLimit_ = true;
  }
  // Handle maxEvents parameter.
  int64_t maxEvents_par = p.get<int64_t>("maxEvents", -1);
  if (maxEvents_par > -1) {
    remainingEvents_ = maxEvents_par;
    haveEventLimit_ = true;
  }
  // Verify we got a real ActivityRegistry.
  if (!act_) { throw Exception(errors::LogicError) << "no ActivityRegistry\n"; }
  // Finish product registration.
  finishProductRegistration_(d);
}

template <class T>
void
art::Source<T>::throwDataCorruption_(const char * msg)
{
  throw Exception(errors::DataCorruption) << msg;
}

template <class T>
void
art::Source<T>::throwIfInsane_(bool const result,
                               RunPrincipal* newR,
                               SubRunPrincipal* newSR,
                               EventPrincipal* newE) const
{
  std::ostringstream errMsg;
  if (result) {
    if (!newR && !newSR && !newE)
      throw Exception(errors::LogicError)
        << "readNext returned true but created no new data\n";
    if (cachedRP_ && newR && cachedRP_.get() == newR) {
      errMsg
        << "readNext returned a new Run which is the old Run for "
        << cachedRP_->id() << ".\nIf you don't have a new run, don't return one!\n";
    }
    if (cachedSRP_ && newSR && cachedSRP_.get() == newSR) {
      errMsg
        << "readNext returned a new SubRun which is the old SubRun for "
        << cachedSRP_->id() << ".\nIf you don't have a new subRun, don't return one!\n";
    }
    // Either or both of the above cases could be true and we need
    // to make both of them safe before we throw:
    if (!errMsg.str().empty())
      throw Exception(errors::LogicError)
        << errMsg.str();
    if (cachedRP_ && cachedSRP_) {
      if (!newR && newSR && newSR->id() == cachedSRP_->id())
        throwDataCorruption_("readNext returned a 'new' SubRun "
                             "that was the same as the previous "
                             "SubRun\n");
      if (newR && newR->id() == cachedRP_->id())
        throwDataCorruption_("readNext returned a 'new' Run "
                             "that was the same as the previous "
                             "Run\n");
      if (newR && !newSR && newE)
        throwDataCorruption_("readNext returned a new Run and "
                             "Event without a SubRun\n");
      if (newR && newSR && newSR->id() == cachedSRP_->id())
        throwDataCorruption_("readNext returned a new Run with "
                             "a SubRun from the wrong Run\n");
    }
    RunID rID;
    SubRunID srID;
    EventID eID;
    if (newR) {
      rID = newR->id();
      if (!rID.isValid()) {
        throwDataCorruption_("readNext returned a Run with an invalid RunID.\n");
      }
    }
    else if (cachedRP_) { rID = cachedRP_->id(); }
    if (newSR) {
      srID = newSR->id();
      if (rID != srID.runID()) {
        errMsg << "readNext returned a SubRun "
               << srID
               << " which is a mismatch to "
               << rID << "\n";
        throwDataCorruption_(errMsg.str().c_str());
      }
      if (!srID.isValid()) {
        throwDataCorruption_("readNext returned a SubRun with an invalid SubRunID.\n");
      }
      if (newSR->runPrincipalExemptPtr()) {
        throwDataCorruption_("readNext returned a SubRun with a non-null embedded Run.\n");
      }
    }
    else if (cachedSRP_) { srID = cachedSRP_->id(); }
    if (newE) {
      eID = newE->id();
      if (srID != eID.subRunID()) {
        errMsg << "readNext returned an Event "
               << eID
               << " which is a mismatch to "
               << srID << "\n";
        throwDataCorruption_(errMsg.str().c_str());
      }
      if (!eID.isValid()) {
        throwDataCorruption_("readNext returned an Event with an invalid EventID.\n");
      }
      if (newE->subRunPrincipalExemptPtr()) {
        throwDataCorruption_("readNext returned an Event with a non-null embedded SubRun.\n");
      }
    }
  }
  else {
    if (newR || newSR || newE)
      throw Exception(errors::LogicError)
          << "readNext returned false but created new data\n";
  }
}

template <class T>
bool
art::Source<T>::readNext_()
{
  std::unique_ptr<RunPrincipal> newR {nullptr};
  std::unique_ptr<SubRunPrincipal> newSR {nullptr};
  std::unique_ptr<EventPrincipal> newE {nullptr};

  bool result {false};
  {
    RunPrincipal* nR {nullptr};
    SubRunPrincipal* nSR {nullptr};
    EventPrincipal* nE {nullptr};
    result = detail_.readNext(cachedRP_.get(), cachedSRP_.get(), nR, nSR, nE);
    newR.reset(nR);
    newSR.reset(nSR);
    newE.reset(nE);
    throwIfInsane_(result, newR.get(), newSR.get(), newE.get());
  }

  if (result) {
    subRunIsNew_ = newSR &&
      ((!cachedSRP_) || newSR->id() != cachedSRP_->id());
    pendingSubRun_ = newSR.get() != nullptr;
    pendingEvent_  = newE.get() != nullptr;
    if (newR) {
      newRP_ = std::move(newR);
    }
    if (newSR) {
      auto rp = newRP_ ? newRP_.get() : cachedRP_.get();
      newSR->setRunPrincipal(rp);
      newSRP_ = std::move(newSR);
    }
    if (newE) {
      auto srp = newSRP_ ? newSRP_.get() : cachedSRP_.get();
      newE->setSubRunPrincipal(srp);
      newE_ = std::move(newE);
    }
    if (newRP_)
    { state_ = input::IsRun; }
    else if (newSRP_)
    { state_ = input::IsSubRun; }
    else if (newE_)
    { state_ = input::IsEvent; }
  }
  return result;
}

template <class T>
void
art::Source<T>::checkForNextFile_()
{
  state_ = input::IsStop; // Default -- may change below.
  if (Source_generator<T>::value) {
    std::conditional_t<detail::has_hasMoreData<T>::value, detail::do_call_hasMoreData<T>, detail::do_not_call_hasMoreData<T>>
      generatorHasMoreData;
    if (generatorHasMoreData(detail_)) {
      state_ = input::IsFile;
    }
  }
  else {
    currentFileName_ = fh_.next();
    if (!currentFileName_.empty()) {
      state_ = input::IsFile;
    }
  }
}

template <class T>
art::input::ItemType
art::Source<T>::nextItemType()
{
  if (remainingEvents_ == 0) { state_ = input::IsStop; }
  switch (state_) {
    case input::IsInvalid:
      if (Source_generator<T>::value) {
        state_ = input::IsFile; // Once.
      }
      else {
        checkForNextFile_();
      }
      break;
    case input::IsFile:
      readNextAndRequireRun_();
      break;
    case input::IsRun:
      if (pendingSubRun_ || (pendingEvent_ && cachedSRP_)) {
        state_ = input::IsSubRun;
        pendingSubRun_ = false;
      }
      else if (pendingEvent_)
        throw Exception(errors::DataCorruption)
            << "Input file '"
            << currentFileName_
            << "' contains an Event "
            << newE_->id()
            << " that belongs to no SubRun\n";
      else
      { readNextAndRequireRun_(); }
      break;
    case input::IsSubRun:
      if (pendingEvent_) {
        state_ = input::IsEvent;
        pendingEvent_ = false;
      }
      else
      { readNextAndRefuseEvent_(); }
      break;
    case input::IsEvent:
      if (!readNext_()) {
        checkForNextFile_();
      }
      break;
    case input::IsStop:
      break;
  }
  if ((state_ == input::IsRun ||
       state_ == input::IsSubRun) &&
      remainingSubRuns_ == 0)
  { state_ = input::IsStop; }
  return state_;
}

template <class T>
void
art::Source<T>::readNextAndRequireRun_()
{
  if (readNext_()) {
    if (state_ != input::IsRun) {
      if (cachedRP_) {
        state_ = input::IsRun; // Regurgitate existing cached run.
      }
      else {
        throw Exception(errors::DataCorruption)
            << "Input file '"
            << currentFileName_
            << "' has a"
            << (state_ == input::IsSubRun ? " SubRun" : "n Event")
            << " where a Run is expected\n";
      }
    }
  }
  else {
    checkForNextFile_();
  }
}

template <class T>
void
art::Source<T>::readNextAndRefuseEvent_()
{
  if (readNext_()) {
    if (state_ == input::IsEvent) {
      throw Exception(errors::DataCorruption)
          << "Input file '"
          << currentFileName_
          << "' has an Event where a Run or SubRun is expected\n";
    }
  }
  else {
    checkForNextFile_();
  }
}

template <class T>
art::RunID
art::Source<T>::run() const
{
  if (!newRP_) throw Exception(errors::LogicError)
        << "Error in Source<T>\n"
        << "run() called when no RunPrincipal exists\n"
        << "Please report this to the art developers\n";
  return newRP_->id();
}

template <class T>
std::unique_ptr<art::RangeSetHandler>
art::Source<T>::runRangeSetHandler()
{
  return std::make_unique<OpenRangeSetHandler>(cachedRP_->run());
}

template <class T>
art::SubRunID
art::Source<T>::subRun() const
{
  if (!newSRP_) throw Exception(errors::LogicError)
        << "Error in Source<T>\n"
        << "subRun() called when no SubRunPrincipal exists\n"
        << "Please report this to the art developers\n";
  return newSRP_->id();
}

template <class T>
std::unique_ptr<art::RangeSetHandler>
art::Source<T>::subRunRangeSetHandler()
{
  return std::make_unique<OpenRangeSetHandler>(cachedSRP_->run());
}



template <class T>
std::unique_ptr<art::FileBlock>
art::Source<T>::readFile(MasterProductRegistry&)
{
  FileBlock* newF {nullptr};
  detail_.readFile(currentFileName_, newF);
  if (!newF) {
    throw Exception(errors::LogicError)
        << "detail_::readFile() failed to return a valid FileBlock object\n";
  }
  return std::unique_ptr<FileBlock>(newF);
}

template <class T>
void
art::Source<T>::closeFile()
{
  detail_.closeCurrentFile();
  // Cached pointers are no longer valid since the PrincipalCache is
  // cleared after file close.
  cachedRP_ = nullptr;
  cachedSRP_ = nullptr;
}

template <class T>
std::unique_ptr<art::RunPrincipal>
art::Source<T>::readRun()
{
  if (!newRP_) throw Exception(errors::LogicError)
        << "Error in Source<T>\n"
        << "readRun() called when no RunPrincipal exists\n"
        << "Please report this to the art developers\n";
  cachedRP_ = newRP_.get();
  return std::move(newRP_);
}

template <class T>
std::unique_ptr<art::SubRunPrincipal>
art::Source<T>::readSubRun(cet::exempt_ptr<RunPrincipal>)
{
  if (!newSRP_) throw Exception(errors::LogicError)
        << "Error in Source<T>\n"
        << "readSubRun() called when no SubRunPrincipal exists\n"
        << "Please report this to the art developers\n";
  if (subRunIsNew_) {
    if (haveSRLimit_) { --remainingSubRuns_; }
    subRunIsNew_ = false;
  }
  cachedSRP_ = newSRP_.get();
  return std::move(newSRP_);
}

template <class T>
std::unique_ptr<art::EventPrincipal>
art::Source<T>::readEvent(cet::exempt_ptr<SubRunPrincipal>)
{
  if (haveEventLimit_) { --remainingEvents_; }
  return std::move(newE_);
}

template <class T>
void
art::Source<T>::finishProductRegistration_(InputSourceDescription& d)
{
  // These _xERROR_ strings should never appear in branch names; they
  // are here as tracers to help identify any failures in coding.
  h_.registerProducts(d.productRegistry,
                      ModuleDescription{fhicl::ParameterSet{}.id(), // Dummy
                                        "_NAMEERROR_",
                                        "_LABELERROR_",
                                        d.moduleDescription.processConfiguration()});
}

#endif /* art_Framework_IO_Sources_Source_h */


// Local Variables:
// mode: c++
// End:
