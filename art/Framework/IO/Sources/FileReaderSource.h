#ifndef art_Framework_IO_Sources_FileReaderSource_h
#define art_Framework_IO_Sources_FileReaderSource_h

// ======================================================================
//
// The FileReaderSource class template is used to create InputSources which
// are capable of reading Runs, SubRuns and Events from non-standard
// input files. Sources instantiated from FileReaderSource are *not* random
// access sources.
//
// The FileReaderSource class template requires the use of a type T as its
// template parameter. The type T must supply the following non-static
// member functions:
//
//    // Construct an object of type T. The ParameterSet provided will
//    // be that constructed by the 'source' statement in the job
//    // configuration file. The ProductRegistryHelper must be used
//    // to register and products to be reconstituted by this source.
//    T(fhicl::ParameterSet const&, art::ProductRegistryHelper& );
//
//    // Open the file of the given name, returning a new fileblock in
//    // fb. If readFile is unable to return a valid FileBlock it
//    // should throw (suggestions for suitable exceptions are:
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

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/PrincipalMaker.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "fhiclcpp/ParameterSet.h"
#include "cetlib/exempt_ptr.h"

#include "boost/shared_ptr.hpp"
#include "boost/noncopyable.hpp"

#include <algorithm>
#include <memory>

// ----------------------------------------------------------------------

namespace art 
{

  template <class T>
  class FileReaderSource : public InputSource, private boost::noncopyable
  {
  public:
    typedef T ReaderDetail;

    FileReaderSource(fhicl::ParameterSet const& p,
                     InputSourceDescription const& d);

    virtual ~FileReaderSource();

    virtual input::ItemType nextItemType();
    virtual RunNumber_t run() const;
    virtual SubRunNumber_t subRun() const;
    virtual boost::shared_ptr<FileBlock> readFile();
    virtual void closeFile();

    virtual boost::shared_ptr<RunPrincipal> readRun();

    virtual boost::shared_ptr<SubRunPrincipal>
    readSubRun(boost::shared_ptr<RunPrincipal> rp);

    virtual std::auto_ptr<EventPrincipal>
    readEvent(boost::shared_ptr<SubRunPrincipal> srp);

  private:
    typedef std::vector<std::string> strings;
    typedef strings::const_iterator  iter;

    cet::exempt_ptr<ProductRegistry>  preg_;
    cet::exempt_ptr<ActivityRegistry> act_;

    ProductRegistryHelper  h_;
    ProcessConfiguration   pc_;
    ReaderDetail           detail_;
    strings                filenames_;
    iter                   currentFile_;
    iter                   end_;
    input::ItemType        state_;

    boost::shared_ptr<RunPrincipal>    cachedRP_;
    boost::shared_ptr<SubRunPrincipal> cachedSRP_;
    std::auto_ptr<EventPrincipal>      cachedE_;

    bool hasNewSubRun_;
    bool hasNewEvent_;

    // Called in the constructor, to finish the process of product
    // registration.
    void finishProductRegistration_(InputSourceDescription const& d);

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
    void throwIfInsane_(bool result, RunPrincipal* newR,
                        SubRunPrincipal* newSR,
                        EventPrincipal* newE) const;

    // Throw an art::Exception(errors::DataCorruption), with the given
    // message text.
    static void throwDataCorruption_(const char* msg);
  };

}  // art

// ======================================================================

namespace art
{
  using fhicl::ParameterSet;

  template <class T>
  FileReaderSource<T>::FileReaderSource(ParameterSet const& p,
                                        InputSourceDescription const& d) :
    InputSource(),
    preg_(d.productRegistry_),
    act_(d.actReg_.get()),
    h_(),
    pc_(d.moduleDescription_.processConfiguration_),
    detail_(p, h_, PrincipalMaker(*preg_, pc_)),
    filenames_(p.get<strings>("fileNames")),
    currentFile_(),
    end_(),
    state_(input::IsInvalid),
    cachedRP_(),
    cachedSRP_(),
    cachedE_(),
    hasNewSubRun_(false),
    hasNewEvent_(false)
  {
    if (!preg_ ) throw Exception(errors::LogicError) << "no ProductRegistry\n";
    if (!act_) throw Exception(errors::LogicError) << "no ActivityRegistry\n";
    finishProductRegistration_(d);
    std::sort(filenames_.begin(),
              filenames_.end());
    currentFile_ = filenames_.begin();
    end_ = filenames_.end();
  }

  template <class T>
  FileReaderSource<T>::~FileReaderSource()
  { }

  template <class T>
  void
  FileReaderSource<T>::throwDataCorruption_(const char* msg)
  {
    throw Exception(errors::DataCorruption) << msg;
  }

  struct cleanup
  {
    RunPrincipal* r;
    SubRunPrincipal* sr;
    EventPrincipal* e;
    void clear() { r=0; sr=0; e=0; }
    ~cleanup() { delete r; delete sr; delete e; }
    cleanup(RunPrincipal* r, SubRunPrincipal* sr, EventPrincipal* e) :
      r(r), sr(sr), e(e) { }
  };


  template <class T>
  void
  FileReaderSource<T>::throwIfInsane_(bool result, RunPrincipal* newR,
                                      SubRunPrincipal* newSR,
                                      EventPrincipal* newE) const
  {
    cleanup sentry(newR, newSR, newE);
    std::ostringstream errMsg;
    if (result)
      {
        if (!newR && !newSR && !newE)
          throw Exception(errors::LogicError)
            << "readNext returned true but created no new data\n";
        if (cachedRP_ && newR && cachedRP_.get() == newR)
          {
            // Avoid double delete with cleanup sentry and statemachine
            // teardown.
            newR = 0;
            errMsg
              << "readNext returned a new Run which is the old Run for "
              << cachedRP_->id() << ".\nIf you don't have a new run, don't return one!\n";
          }
        if (cachedSRP_ && newSR && cachedSRP_.get() == newSR)
          {
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
        if (cachedRP_ && cachedSRP_)
          {
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
        if (newR)
          {
          rID = newR->id();
          if (!rID.isValid())
            {
              throwDataCorruption_("readNext returned a Run with an invalid RunID.\n");
            }
          }
        else if (cachedRP_) rID = cachedRP_->id();
        if (newSR)
          {
            srID = newSR->id();
            if (rID != srID.runID())
              {
                errMsg << "readNext returned a SubRun "
                       << srID
                       << " which is a mismatch to "
                       << rID << "\n";
                throwDataCorruption_(errMsg.str().c_str());
              }
            if (!srID.isValid())
              {
                throwDataCorruption_("readNext returned a SubRun with an invalid SubRunID.\n");
              }
          }
        else if (cachedSRP_) srID = cachedSRP_->id();
        if (newE)
          {
            eID = newE->id();
            if (srID != eID.subRunID())
              {
                errMsg << "readNext returned an Event "
                       << eID
                       << " which is a mismatch to "
                       << srID << "\n";
                throwDataCorruption_(errMsg.str().c_str());
              }
            if (!eID.isValid())
              {
                throwDataCorruption_("readNext returned an Event with an invalid EventID.\n");
              }
          }
      }
    else
      {
        if (newR || newSR || newE)
          throw Exception(errors::LogicError)
            << "readNext returned false but created new data\n";
      }
    sentry.clear();
  }

  template <class T>
  bool
  FileReaderSource<T>::readNext_()
  {
    RunPrincipal* newR = 0;
    SubRunPrincipal* newSR = 0;
    EventPrincipal* newE = 0;
    bool result = detail_.readNext(cachedRP_.get(), 
                                   cachedSRP_.get(), 
                                   newR, newSR, newE);    
    throwIfInsane_(result, newR, newSR, newE);

    if (result)
      {
        hasNewSubRun_ = newSR;
        hasNewEvent_  = newE;

        if (newR)  cachedRP_.reset(newR);
        if (newSR) cachedSRP_.reset(newSR);
        cachedE_.reset(newE);

        if (newR) 
          state_ = input::IsRun;
        else if (newSR)
          state_ = input::IsSubRun;
        else if (cachedE_.get()) 
          state_ = input::IsEvent;
      }

    return result;
  }

  template <class T>
  void
  FileReaderSource<T>::checkForNextFile_()
  {
    if (currentFile_ == end_ || ++currentFile_ == end_) {
      state_ = input::IsStop;
    } else {
      state_ = input::IsFile;
    }
  }

  template <class T>
  input::ItemType
  FileReaderSource<T>::nextItemType()
  {
    switch (state_)
      {
      case input::IsInvalid:
        if (currentFile_ == end_) {
          state_ = input::IsStop;
        } else {
          state_ = input::IsFile;
        }
        break;

      case input::IsFile:
        readNextAndRequireRun_();
        break;

      case input::IsRun:
        if (hasNewSubRun_)
          state_ = input::IsSubRun;
        else if(hasNewEvent_)
          throw Exception(errors::DataCorruption)
            << "Input file '"
            << *currentFile_
            << "' contains an Event "
            << cachedE_->id()
            << " that belongs to no SubRun\n";
        else
          readNextAndRequireRun_();
        break;

      case input::IsSubRun:
        if (hasNewEvent_)
          state_ = input::IsEvent;
        else 
          readNextAndRefuseEvent_();
        break;

      case input::IsEvent:
        if (!readNext_())
          {
            checkForNextFile_();
          }
        break;

      case input::IsStop:
        break;
      }
    return state_;
  }

  template <class T>
  void
  FileReaderSource<T>::readNextAndRequireRun_()
  {
    if (readNext_())
      { 
        if (state_ != input::IsRun) 
          throw Exception(errors::DataCorruption)
            << "Input file '"
            << *currentFile_
            << "' has a"
            << (state_ == input::IsSubRun ? " SubRun" : "n Event")
            << " where a Run is expected\n";
      }
    else
      {
        checkForNextFile_();
      }
  }    

  template <class T>
  void
  FileReaderSource<T>::readNextAndRefuseEvent_()
  {
    if (readNext_())
      { 
        if (state_ == input::IsEvent) 
          throw Exception(errors::DataCorruption)
            << "Input file '"
            << *currentFile_
            << "' has an Event where a Run or SubRun is expected\n";
      }
    else
      {
        checkForNextFile_();
      }
  }

  template <class T>
  RunNumber_t
  FileReaderSource<T>::run() const
  {
    if (!cachedRP_) throw Exception(errors::LogicError)
      << "Error in FileReaderSource<T>\n"
      << "run() called when no RunPrincipal exists\n"
      << "Please report this to the art developers\n";
    return cachedRP_->id().run();
  }

  template <class T>
  SubRunNumber_t
  FileReaderSource<T>::subRun() const
  {
    if (!cachedSRP_) throw Exception(errors::LogicError)
      << "Error in FileReaderSource<T>\n"
      << "subRun() called when no SubRunPrincipal exists\n"
      << "Please report this to the art developers\n";
    return cachedSRP_->id().subRun();
  }

  template <class T>
  boost::shared_ptr<FileBlock>
  FileReaderSource<T>::readFile()
  {
    FileBlock* newF = 0;
    detail_.readFile(*currentFile_, newF);
    if (!newF)
      {
        throw Exception(errors::LogicError)
          << "detail_::readFile() failed to return a valid FileBlock object\n";
      }
    return boost::shared_ptr<FileBlock>(newF);
  }

  template <class T>
  void
  FileReaderSource<T>::closeFile()
  {
    detail_.closeCurrentFile();
  }

  template <class T>
  boost::shared_ptr<RunPrincipal>
  FileReaderSource<T>::readRun()
  {
    if (!cachedRP_) throw Exception(errors::LogicError)
      << "Error in FileReaderSource<T>\n"
      << "readRun() called when no RunPrincipal exists\n"
      << "Please report this to the art developers\n";
    return cachedRP_;
  }

  template <class T>
  boost::shared_ptr<SubRunPrincipal>
  FileReaderSource<T>::readSubRun(boost::shared_ptr<RunPrincipal> rp)
  {
    if (!cachedSRP_) throw Exception(errors::LogicError)
      << "Error in FileReaderSource<T>\n"
      << "readSubRun() called when no SubRunPrincipal exists\n"
      << "Please report this to the art developers\n";
    return cachedSRP_;
  }

  template <class T>
  std::auto_ptr<EventPrincipal>
  FileReaderSource<T>::readEvent(boost::shared_ptr<SubRunPrincipal> srp)
  {
    return cachedE_;
  }

  template <class T>
  void
  FileReaderSource<T>::finishProductRegistration_(InputSourceDescription
                                                  const& d)
  {
    ModuleDescription md;
    // These _xERROR_ strings should never appear in branch names; they
    // are here as tracers to help identify any failures in coding.
    md.moduleName_ = "_NAMEERROR_";
    md.moduleLabel_ = "_LABELERROR_";
    md.processConfiguration_.processName_ = "_PROCESSERROR_";
    md.parameterSetID_ = fhicl::ParameterSet().id(); // Dummy

    TypeLabelList& types = h_.typeLabelList();
    ProductRegistry const& reg = *d.productRegistry_;
    ProductRegistryHelper::addToRegistry(types.begin(),
                                         types.end(),
                                         md,
                                         const_cast<ProductRegistry&>(reg),
                                         false);
  }

}

#endif /* art_Framework_IO_Sources_FileReaderSource_h */


// Local Variables:
// mode: c++
// End:
