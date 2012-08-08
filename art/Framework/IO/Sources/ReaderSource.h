#ifndef art_Framework_IO_Sources_ReaderSource_h
#define art_Framework_IO_Sources_ReaderSource_h

// ======================================================================
//
// The ReaderSource class template is used to create InputSources which
// are capable of reading Runs, SubRuns and Events from non-standard
// input files. Sources instantiated from ReaderSource are *not* random
// access sources.
//
// The ReaderSource class template requires the use of a type T as its
// template parameter. The type T must supply the following non-static
// member functions:
//
//    // Construct an object of type T. The ParameterSet provided will
//    // be that constructed by the 'source' statement in the job
//    // configuration file. The ProductRegistryHelper must be used
//    // to register products to be reconstituted by this source.
//    T(fhicl::ParameterSet const &,
//      art::ProductRegistryHelper &,
//      art::PrincipalMaker const &);
//
//    // Open the file of the given name, returning a new fileblock in
//    // fb. If readFile is unable to return a valid FileBlock it
//    // should throw. Suggestions for suitable exceptions are:
//    // art::Exception(art::errors::FileOpenError) or
//    // art::Exception(art::errors::FileReadError).
//    void readFile(std::string const& filename,
//                  art::FileBlock*& fb);
//
//    // Read the next part of the current file. Return false if nothing
//    // was read; return true and set the appropriate 'out' arguments
//    // if something was read.
//    bool readNext(art::RunPrincipal* const& inR,
//                  art::SubRunPrincipal* const& inSR,
//                  art::RunPrincipal*& outR,
//                  art::SubRunPrincipal*& outSR,
//                  art::EventPrincipal*& outE)
//
//    // Close the current input file.
//    void closeCurrentFile();
//
// ======================================================================

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/PrincipalMaker.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "boost/noncopyable.hpp"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/algorithm"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

#include <limits>

// ----------------------------------------------------------------------

namespace art {

  template <class T>
  class ReaderSource : public InputSource, private boost::noncopyable {
  public:
    typedef T ReaderDetail;

    ReaderSource(fhicl::ParameterSet const & p,
                 InputSourceDescription & d);

    virtual ~ReaderSource();

    virtual input::ItemType nextItemType();
    virtual RunNumber_t run() const;
    virtual SubRunNumber_t subRun() const;

    // ReaderSource does not use the MasterProductRegistry it is passed.
    // This *could* be used for merging files read by a ReaderSource;
    // however, doing so requires solving some hard problems of file
    // merging in a more general manner than has been done thus far.
    virtual std::shared_ptr<FileBlock> readFile(MasterProductRegistry &);
    virtual void closeFile();

    virtual std::shared_ptr<RunPrincipal> readRun();

    virtual std::shared_ptr<SubRunPrincipal>
    readSubRun(std::shared_ptr<RunPrincipal> rp);

    using InputSource::readEvent;
    virtual std::unique_ptr<EventPrincipal>
    readEvent(std::shared_ptr<SubRunPrincipal> srp);

  private:
    typedef std::vector<std::string> strings;
    typedef strings::const_iterator  iter;

    cet::exempt_ptr<ActivityRegistry> act_;

    ProductRegistryHelper  h_;
    ProcessConfiguration   pc_;
    PrincipalMaker         principalMaker_; // So it can be used by detail.
    ReaderDetail           detail_;
    strings                filenames_;
    iter                   currentFile_;
    iter                   end_;
    input::ItemType        state_;

    std::shared_ptr<RunPrincipal> cachedRP_;
    std::shared_ptr<SubRunPrincipal> cachedSRP_;
    std::unique_ptr<EventPrincipal> cachedE_;

    bool pendingSubRun_;
    bool pendingEvent_;

    bool subRunIsNew_;

    SubRunNumber_t remainingSubRuns_;
    bool haveSRLimit_;
    EventNumber_t remainingEvents_;
    bool haveEventLimit_;

    // Called in the constructor, to finish the process of product
    // registration.
    void finishProductRegistration_(InputSourceDescription & d);

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
    void throwIfInsane_(bool result, RunPrincipal * newR,
                        SubRunPrincipal * newSR,
                        EventPrincipal * newE) const;

    // Throw an art::Exception(errors::DataCorruption), with the given
    // message text.
    static void throwDataCorruption_(const char * msg);
  };

}  // art

// ======================================================================

namespace art {
  using fhicl::ParameterSet;

  template <class T>
  ReaderSource<T>::ReaderSource(ParameterSet const & p,
                                InputSourceDescription & d) :
    InputSource(),
    act_(&d.activityRegistry),
    h_(),
    pc_(d.moduleDescription.processConfiguration_),
    principalMaker_(pc_),
    detail_(p, h_, principalMaker_),
    filenames_(p.get<strings>("fileNames")),
    currentFile_(),
    end_(),
    state_(input::IsInvalid),
    cachedRP_(),
    cachedSRP_(),
    cachedE_(),
    pendingSubRun_(false),
    pendingEvent_(false),
    subRunIsNew_(false),
    remainingSubRuns_(1),
    haveSRLimit_(false),
    remainingEvents_(1),
    haveEventLimit_(false)
  {
    int64_t maxSubRuns_par = p.get<int64_t>("maxSubRuns", -1);
    if (maxSubRuns_par > -1) {
      remainingSubRuns_ = maxSubRuns_par;
      haveSRLimit_ = true;
    }
    int64_t maxEvents_par = p.get<int64_t>("maxEvents", -1);
    if (maxEvents_par > -1) {
      remainingEvents_ = maxEvents_par;
      haveEventLimit_ = true;
    }
    if (!act_) { throw Exception(errors::LogicError) << "no ActivityRegistry\n"; }
    finishProductRegistration_(d);
    std::sort(filenames_.begin(),
              filenames_.end());
    currentFile_ = filenames_.begin();
    end_ = filenames_.end();
  }

  template <class T>
  ReaderSource<T>::~ReaderSource()
  { }

  template <class T>
  void
  ReaderSource<T>::throwDataCorruption_(const char * msg)
  {
    throw Exception(errors::DataCorruption) << msg;
  }

  struct cleanup {
    RunPrincipal * r;
    SubRunPrincipal * sr;
    EventPrincipal * e;
    void clear() { r = 0; sr = 0; e = 0; }
    ~cleanup() { delete r; delete sr; delete e; }
    cleanup(RunPrincipal * r, SubRunPrincipal * sr, EventPrincipal * e) :
      r(r), sr(sr), e(e) { }
  };


  template <class T>
  void
  ReaderSource<T>::throwIfInsane_(bool result, RunPrincipal * newR,
                                  SubRunPrincipal * newSR,
                                  EventPrincipal * newE) const
  {
    cleanup sentry(newR, newSR, newE);
    std::ostringstream errMsg;
    if (result) {
      if (!newR && !newSR && !newE)
        throw Exception(errors::LogicError)
            << "readNext returned true but created no new data\n";
      if (cachedRP_ && newR && cachedRP_.get() == newR) {
        // Avoid double delete with cleanup sentry and statemachine
        // teardown.
        newR = 0;
        errMsg
            << "readNext returned a new Run which is the old Run for "
            << cachedRP_->id() << ".\nIf you don't have a new run, don't return one!\n";
      }
      if (cachedSRP_ && newSR && cachedSRP_.get() == newSR) {
        // Avoid double delete with cleanup sentry and statemachine
        // teardown.
        newSR = 0;
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
        if (newSR->runPrincipalSharedPtr()) {
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
        if (newE->subRunPrincipalSharedPtr()) {
          throwDataCorruption_("readNext returned an Event with a non-null embedded SubRun.\n");
        }
      }
    }
    else {
      if (newR || newSR || newE)
        throw Exception(errors::LogicError)
            << "readNext returned false but created new data\n";
    }
    sentry.clear();
  }

  template <class T>
  bool
  ReaderSource<T>::readNext_()
  {
    RunPrincipal * newR = 0;
    SubRunPrincipal * newSR = 0;
    EventPrincipal * newE = 0;
    bool result = detail_.readNext(cachedRP_.get(),
                                   cachedSRP_.get(),
                                   newR, newSR, newE);
    throwIfInsane_(result, newR, newSR, newE);
    if (result) {
      subRunIsNew_ = newSR &&
                     ((!cachedSRP_) || newSR->id() != cachedSRP_->id());
      pendingSubRun_ = newSR;
      pendingEvent_  = newE;
      if (newR) { cachedRP_.reset(newR); }
      if (newSR) {
        newSR->setRunPrincipal(cachedRP_);
        cachedSRP_.reset(newSR);
      }
      newE->setSubRunPrincipal(cachedSRP_);
      cachedE_.reset(newE);
      if (newR)
      { state_ = input::IsRun; }
      else if (newSR)
      { state_ = input::IsSubRun; }
      else if (cachedE_.get())
      { state_ = input::IsEvent; }
    }
    return result;
  }

  template <class T>
  void
  ReaderSource<T>::checkForNextFile_()
  {
    if (currentFile_ == end_ || ++currentFile_ == end_) {
      state_ = input::IsStop;
    }
    else {
      state_ = input::IsFile;
    }
  }

  template <class T>
  input::ItemType
  ReaderSource<T>::nextItemType()
  {
    if (remainingEvents_ == 0) { state_ = input::IsStop; }
    switch (state_) {
      case input::IsInvalid:
        if (currentFile_ == end_) {
          state_ = input::IsStop;
        }
        else {
          state_ = input::IsFile;
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
              << *currentFile_
              << "' contains an Event "
              << cachedE_->id()
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
  ReaderSource<T>::readNextAndRequireRun_()
  {
    if (readNext_()) {
      if (state_ != input::IsRun) {
        if (cachedRP_) {
          state_ = input::IsRun; // Regurgitate exsting cached run.
        }
        else {
          throw Exception(errors::DataCorruption)
              << "Input file '"
              << *currentFile_
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
  ReaderSource<T>::readNextAndRefuseEvent_()
  {
    if (readNext_()) {
      if (state_ == input::IsEvent)
        throw Exception(errors::DataCorruption)
            << "Input file '"
            << *currentFile_
            << "' has an Event where a Run or SubRun is expected\n";
    }
    else {
      checkForNextFile_();
    }
  }

  template <class T>
  RunNumber_t
  ReaderSource<T>::run() const
  {
    if (!cachedRP_) throw Exception(errors::LogicError)
          << "Error in ReaderSource<T>\n"
          << "run() called when no RunPrincipal exists\n"
          << "Please report this to the art developers\n";
    return cachedRP_->id().run();
  }

  template <class T>
  SubRunNumber_t
  ReaderSource<T>::subRun() const
  {
    if (!cachedSRP_) throw Exception(errors::LogicError)
          << "Error in ReaderSource<T>\n"
          << "subRun() called when no SubRunPrincipal exists\n"
          << "Please report this to the art developers\n";
    return cachedSRP_->id().subRun();
  }

  template <class T>
  std::shared_ptr<FileBlock>
  ReaderSource<T>::readFile(MasterProductRegistry &)
  {
    FileBlock * newF = 0;
    detail_.readFile(*currentFile_, newF);
    if (!newF) {
      throw Exception(errors::LogicError)
          << "detail_::readFile() failed to return a valid FileBlock object\n";
    }
    return std::shared_ptr<FileBlock>(newF);
  }

  template <class T>
  void
  ReaderSource<T>::closeFile()
  {
    detail_.closeCurrentFile();
  }

  template <class T>
  std::shared_ptr<RunPrincipal>
  ReaderSource<T>::readRun()
  {
    if (!cachedRP_) throw Exception(errors::LogicError)
          << "Error in ReaderSource<T>\n"
          << "readRun() called when no RunPrincipal exists\n"
          << "Please report this to the art developers\n";
    return cachedRP_;
  }

  template <class T>
  std::shared_ptr<SubRunPrincipal>
  ReaderSource<T>::readSubRun(std::shared_ptr<RunPrincipal>)
  {
    if (!cachedSRP_) throw Exception(errors::LogicError)
          << "Error in ReaderSource<T>\n"
          << "readSubRun() called when no SubRunPrincipal exists\n"
          << "Please report this to the art developers\n";
    if (subRunIsNew_) {
      if (haveSRLimit_) { --remainingSubRuns_; }
      subRunIsNew_ = false;
    }
    return cachedSRP_;
  }

  template <class T>
  std::unique_ptr<EventPrincipal>
  ReaderSource<T>::readEvent(std::shared_ptr<SubRunPrincipal>)
  {
    if (haveEventLimit_) { --remainingEvents_; }
    return cachedE_;
  }

  template <class T>
  void
  ReaderSource<T>::finishProductRegistration_(InputSourceDescription & d)
  {
    ModuleDescription md;
    // These _xERROR_ strings should never appear in branch names; they
    // are here as tracers to help identify any failures in coding.
    md.moduleName_ = "_NAMEERROR_";
    md.moduleLabel_ = "_LABELERROR_";
    md.processConfiguration_.processName_ = d.moduleDescription.processConfiguration_.processName_;
    md.parameterSetID_ = fhicl::ParameterSet().id(); // Dummy
    h_.registerProducts(d.productRegistry, md);
  }
}

#endif /* art_Framework_IO_Sources_ReaderSource_h */


// Local Variables:
// mode: c++
// End:
