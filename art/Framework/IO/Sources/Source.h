#ifndef art_Framework_IO_Sources_Source_h
#define art_Framework_IO_Sources_Source_h
// vim: set sw=2 expandtab :

// ======================================================================
//
// The Source class template is used to create InputSources which
// are capable of reading Runs, SubRuns and Events from non-standard
// input files. Sources instantiated from Source are *not* random
// access sources.
//
// The Source class template requires the use of a type T as its
// template parameter, satisfying the conditions outlined below. In
// one's XXX_module.cc class one must provide a type alias and module macro
// call along the lines of:
//
// namespace arttest {
//   using GeneratorTest = art::Source<GeneratorTestDetail>;
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
// the type alias above which will attempt to instantiate them. See
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
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/UpdateOutputCallbacks.h"
#include "art/Framework/Core/detail/ImplicitConfigs.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/IO/Sources/SourceHelper.h"
#include "art/Framework/IO/Sources/SourceTraits.h"
#include "art/Framework/IO/Sources/detail/FileNamesHandler.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/TableFragment.h"

#include <algorithm>
#include <concepts>
#include <memory>
#include <type_traits>

namespace art {

  template <typename T>
  class Source;

  template <typename T>
  class SourceTable {
  public:
    using value_type = T;

    auto const&
    operator()() const
    {
      return fragment_();
    }

  private:
    fhicl::TableFragment<T> fragment_;
  };

  namespace detail {

    template <typename T>
    concept has_hasMoreData = requires(T& t) {
                                {
                                  t.hasMoreData()
                                  } -> std::same_as<bool>;
                              };

    template <typename T>
    struct do_call_hasMoreData {
      bool
      operator()(T& t)
      {
        return t.hasMoreData();
      }
    };

    template <typename T>
    struct do_not_call_hasMoreData {
      bool
      operator()(T&)
      {
        return false;
      }
    };

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a Parameters type?
    template <typename T, typename = void>
    struct maybe_has_Parameters : std::false_type {
      using Parameters = fhicl::ParameterSet;
    };

    template <typename T>
    struct maybe_has_Parameters<T, std::void_t<typename T::Parameters>>
      : std::true_type {
      using user_config_t = typename T::Parameters;
      struct Config {
        struct SourceConfig {
          fhicl::Atom<std::string> type{ModuleConfig::plugin_type()};
          fhicl::Sequence<std::string> fileNames{fhicl::Name("fileNames"), {}};
          fhicl::Atom<int64_t> maxSubRuns{fhicl::Name("maxSubRuns"), -1};
          fhicl::Atom<int64_t> maxEvents{fhicl::Name("maxEvents"), -1};
        };
        fhicl::TableFragment<SourceConfig> sourceConfig;
        user_config_t userConfig;
      };
      using Parameters = fhicl::WrappedTable<Config, ModuleConfig::IgnoreKeys>;
    };

  } // namespace detail

  // No-one gets to override this class.
  template <typename T>
  class Source final : public InputSource {
  public:
    using Parameters = typename detail::maybe_has_Parameters<T>::Parameters;

    template <typename U = Parameters>
    explicit Source(std::enable_if_t<std::is_same_v<U, fhicl::ParameterSet>,
                                     fhicl::ParameterSet> const& p,
                    InputSourceDescription& d);

    template <typename U = Parameters>
    explicit Source(
      std::enable_if_t<!std::is_same_v<U, fhicl::ParameterSet>, U> const& p,
      InputSourceDescription& d);

    Source(Source<T> const&) = delete;
    Source(Source<T>&&) = delete;

    Source<T>& operator=(Source<T> const&) = delete;
    Source<T>& operator=(Source<T>&&) = delete;

  private:
    input::ItemType nextItemType() override;
    std::unique_ptr<FileBlock> readFile() override;
    void closeFile() override;

    using InputSource::readEvent;

    std::unique_ptr<RunPrincipal> readRun() override;
    std::unique_ptr<SubRunPrincipal> readSubRun(
      cet::exempt_ptr<RunPrincipal const> rp) override;

    std::unique_ptr<EventPrincipal> readEvent(
      cet::exempt_ptr<SubRunPrincipal const> srp) override;

    std::unique_ptr<RangeSetHandler> runRangeSetHandler() override;
    std::unique_ptr<RangeSetHandler> subRunRangeSetHandler() override;

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
    void throwIfInsane_(bool result,
                        RunPrincipal* newR,
                        SubRunPrincipal* newSR,
                        EventPrincipal* newE) const;

    // Throw an Exception(errors::DataCorruption), with the given
    // message text.
    [[noreturn]] static void throwDataCorruption_(const char* msg);

    ProductRegistryHelper h_{product_creation_mode::reconstitutes};
    UpdateOutputCallbacks& outputCallbacks_;
    ProductTables presentProducts_{ProductTables::invalid()};

    // So it can be used by detail.
    SourceHelper sourceHelper_;
    T detail_;
    input::ItemType state_{input::IsInvalid};
    detail::FileNamesHandler<Source_wantFileServices<T>::value> fh_;
    std::string currentFileName_{};

    std::unique_ptr<RunPrincipal> newRP_{};
    std::unique_ptr<SubRunPrincipal> newSRP_{};
    std::unique_ptr<EventPrincipal> newE_{};

    // Cached Run and SubRun Principals used for users creating new
    // SubRun and Event Principals.  These are non owning!
    cet::exempt_ptr<RunPrincipal> cachedRP_{nullptr};
    cet::exempt_ptr<SubRunPrincipal> cachedSRP_{nullptr};

    bool pendingSubRun_{false};
    bool pendingEvent_{false};
    bool subRunIsNew_{false};
    SubRunNumber_t remainingSubRuns_{1};
    bool haveSRLimit_{false};
    EventNumber_t remainingEvents_{1};
    bool haveEventLimit_{false};
  };

  template <typename T>
  template <typename U>
  Source<T>::Source(std::enable_if_t<std::is_same_v<U, fhicl::ParameterSet>,
                                     fhicl::ParameterSet> const& p,
                    InputSourceDescription& d)
    : InputSource{d.moduleDescription}
    , outputCallbacks_{d.productRegistry}
    , sourceHelper_{d.moduleDescription}
    , detail_{p, h_, sourceHelper_}
    , fh_{p.template get<std::vector<std::string>>("fileNames", {})}
  {
    int64_t const maxSubRuns_par = p.template get<int64_t>("maxSubRuns", -1);
    if (maxSubRuns_par > -1) {
      remainingSubRuns_ = maxSubRuns_par;
      haveSRLimit_ = true;
    }
    int64_t const maxEvents_par = p.template get<int64_t>("maxEvents", -1);
    if (maxEvents_par > -1) {
      remainingEvents_ = maxEvents_par;
      haveEventLimit_ = true;
    }
    finishProductRegistration_(d);
  }

  template <typename T>
  template <typename U>
  Source<T>::Source(
    std::enable_if_t<!std::is_same_v<U, fhicl::ParameterSet>, U> const& p,
    InputSourceDescription& d)
    : InputSource{d.moduleDescription}
    , outputCallbacks_{d.productRegistry}
    , sourceHelper_{d.moduleDescription}
    , detail_{p().userConfig, h_, sourceHelper_}
    , fh_{p().sourceConfig().fileNames()}
  {
    if (int64_t const maxSubRuns_par = p().sourceConfig().maxSubRuns();
        maxSubRuns_par > -1) {
      remainingSubRuns_ = maxSubRuns_par;
      haveSRLimit_ = true;
    }
    if (int64_t const maxEvents_par = p().sourceConfig().maxEvents();
        maxEvents_par > -1) {
      remainingEvents_ = maxEvents_par;
      haveEventLimit_ = true;
    }
    finishProductRegistration_(d);
  }

  template <typename T>
  void
  Source<T>::throwDataCorruption_(const char* msg)
  {
    throw Exception(errors::DataCorruption) << msg;
  }

  template <typename T>
  void
  Source<T>::throwIfInsane_(bool const result,
                            RunPrincipal* newR,
                            SubRunPrincipal* newSR,
                            EventPrincipal* newE) const
  {
    std::ostringstream errMsg;
    if (result) {
      if (!newR && !newSR && !newE) {
        throw Exception{errors::LogicError}
          << "readNext returned true but created no new data\n";
      }
      if (!cachedRP_ && !newR) {
        throw Exception(errors::LogicError)
          << "readNext returned true but no RunPrincipal has been set, and no "
             "cached RunPrincipal exists.\n"
             "This can happen if a new input file has been opened and the "
             "RunPrincipal has not been appropriately assigned.";
      }
      if (!cachedSRP_ && !newSR) {
        throw Exception(errors::LogicError)
          << "readNext returned true but no SubRunPrincipal has been set, and "
             "no cached SubRunPrincipal exists.\n"
             "This can happen if a new input file has been opened and the "
             "SubRunPrincipal has not been appropriately assigned.";
      }
      if (cachedRP_ && newR && cachedRP_.get() == newR) {
        errMsg << "readNext returned a new Run which is the old Run for "
               << cachedRP_->runID()
               << ".\nIf you don't have a new run, don't return one!\n";
      }
      if (cachedSRP_ && newSR && cachedSRP_.get() == newSR) {
        errMsg << "readNext returned a new SubRun which is the old SubRun for "
               << cachedSRP_->subRunID()
               << ".\nIf you don't have a new subRun, don't return one!\n";
      }
      // Either or both of the above cases could be true and we need
      // to make both of them safe before we throw:
      if (!errMsg.str().empty())
        throw Exception(errors::LogicError) << errMsg.str();
      if (cachedRP_ && cachedSRP_) {
        if (!newR && newSR && newSR->subRunID() == cachedSRP_->subRunID())
          throwDataCorruption_("readNext returned a 'new' SubRun "
                               "that was the same as the previous "
                               "SubRun\n");
        if (newR && newR->runID() == cachedRP_->runID())
          throwDataCorruption_("readNext returned a 'new' Run "
                               "that was the same as the previous "
                               "Run\n");
        if (newR && !newSR && newE)
          throwDataCorruption_("readNext returned a new Run and "
                               "Event without a SubRun\n");
        if (newR && newSR && newSR->subRunID() == cachedSRP_->subRunID())
          throwDataCorruption_("readNext returned a new Run with "
                               "a SubRun from the wrong Run\n");
      }
      RunID rID;
      SubRunID srID;
      EventID eID;
      if (newR) {
        rID = newR->runID();
        if (!rID.isValid()) {
          throwDataCorruption_(
            "readNext returned a Run with an invalid RunID.\n");
        }
      } else if (cachedRP_) {
        rID = cachedRP_->runID();
      }
      if (newSR) {
        srID = newSR->subRunID();
        if (rID != srID.runID()) {
          errMsg << "readNext returned a SubRun " << srID
                 << " which is a mismatch to " << rID << '\n';
          throwDataCorruption_(errMsg.str().c_str());
        }
        if (!srID.isValid()) {
          throwDataCorruption_(
            "readNext returned a SubRun with an invalid SubRunID.\n");
        }
      } else if (cachedSRP_) {
        srID = cachedSRP_->subRunID();
      }
      if (newE) {
        eID = newE->eventID();
        if (srID != eID.subRunID()) {
          errMsg << "readNext returned an Event " << eID
                 << " which is a mismatch to " << srID << '\n';
          throwDataCorruption_(errMsg.str().c_str());
        }
        if (!eID.isValid()) {
          throwDataCorruption_(
            "readNext returned an Event with an invalid EventID.\n");
        }
      }
    } else {
      if (newR || newSR || newE)
        throw Exception(errors::LogicError)
          << "readNext returned false but created new data\n";
    }
  }

  template <typename T>
  bool
  Source<T>::readNext_()
  {
    std::unique_ptr<RunPrincipal> newR{nullptr};
    std::unique_ptr<SubRunPrincipal> newSR{nullptr};
    std::unique_ptr<EventPrincipal> newE{nullptr};
    bool result{false};
    {
      RunPrincipal* nR{nullptr};
      SubRunPrincipal* nSR{nullptr};
      EventPrincipal* nE{nullptr};
      result = detail_.readNext(cachedRP_.get(), cachedSRP_.get(), nR, nSR, nE);
      newR.reset(nR);
      newSR.reset(nSR);
      newE.reset(nE);
      throwIfInsane_(result, newR.get(), newSR.get(), newE.get());
    }
    if (result) {
      subRunIsNew_ =
        newSR && ((!cachedSRP_) || newSR->subRunID() != cachedSRP_->subRunID());
      pendingSubRun_ = newSR.get() != nullptr;
      pendingEvent_ = newE.get() != nullptr;
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
      if (newRP_) {
        state_ = input::IsRun;
      } else if (newSRP_) {
        state_ = input::IsSubRun;
      } else if (newE_) {
        state_ = input::IsEvent;
      }
    }
    return result;
  }

  template <typename T>
  void
  Source<T>::checkForNextFile_()
  {
    state_ = input::IsStop; // Default -- may change below.
    if (Source_generator<T>::value) {
      std::conditional_t<detail::has_hasMoreData<T>,
                         detail::do_call_hasMoreData<T>,
                         detail::do_not_call_hasMoreData<T>>
        generatorHasMoreData;
      if (generatorHasMoreData(detail_)) {
        state_ = input::IsFile;
      }
    } else {
      currentFileName_ = fh_.next();
      if (!currentFileName_.empty()) {
        state_ = input::IsFile;
      }
    }
  }

  template <typename T>
  input::ItemType
  Source<T>::nextItemType()
  {
    if (remainingEvents_ == 0) {
      state_ = input::IsStop;
    }
    switch (state_) {
    case input::IsInvalid:
      if (Source_generator<T>::value) {
        state_ = input::IsFile; // Once.
      } else {
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
      } else if (pendingEvent_)
        throw Exception(errors::DataCorruption)
          << "Input file '" << currentFileName_ << "' contains an Event "
          << newE_->eventID() << " that belongs to no SubRun\n";
      else {
        readNextAndRequireRun_();
      }
      break;
    case input::IsSubRun:
      if (pendingEvent_) {
        state_ = input::IsEvent;
        pendingEvent_ = false;
      } else {
        readNextAndRefuseEvent_();
      }
      break;
    case input::IsEvent:
      if (!readNext_()) {
        checkForNextFile_();
      }
      break;
    case input::IsStop:
      break;
    }
    if ((state_ == input::IsRun || state_ == input::IsSubRun) &&
        remainingSubRuns_ == 0) {
      state_ = input::IsStop;
    }
    if (state_ == input::IsStop) {
      // FIXME: upon the advent of a catalog system which can do something
      // intelligent with the difference between whole-file success,
      // partial-file success, partial-file failure and whole-file failure
      // (such as file-open failure), we will need to communicate that
      // difference here. The file disposition options as they are now
      // (and the mapping to any concrete implementation we are are aware
      // of currently) are not sufficient to the task, so we deliberately
      // do not distinguish here between partial-file and whole-file
      // success in particular.
      fh_.finish();
    }
    return state_;
  }

  template <typename T>
  void
  Source<T>::readNextAndRequireRun_()
  {
    if (readNext_()) {
      if (state_ != input::IsRun) {
        if (cachedRP_) {
          state_ = input::IsRun; // Regurgitate existing cached run.
        } else {
          throw Exception(errors::DataCorruption)
            << "Input file '" << currentFileName_ << "' has a"
            << (state_ == input::IsSubRun ? " SubRun" : "n Event")
            << " where a Run is expected\n";
        }
      }
    } else {
      checkForNextFile_();
    }
  }

  template <typename T>
  void
  Source<T>::readNextAndRefuseEvent_()
  {
    if (readNext_()) {
      if (state_ == input::IsEvent) {
        throw Exception(errors::DataCorruption)
          << "Input file '" << currentFileName_
          << "' has an Event where a Run or SubRun is expected\n";
      }
    } else {
      checkForNextFile_();
    }
  }

  template <typename T>
  std::unique_ptr<RangeSetHandler>
  Source<T>::runRangeSetHandler()
  {
    return std::make_unique<OpenRangeSetHandler>(cachedRP_->run());
  }

  template <typename T>
  std::unique_ptr<RangeSetHandler>
  Source<T>::subRunRangeSetHandler()
  {
    return std::make_unique<OpenRangeSetHandler>(cachedSRP_->run());
  }

  template <typename T>
  std::unique_ptr<FileBlock>
  Source<T>::readFile()
  {
    FileBlock* newF{nullptr};
    detail_.readFile(currentFileName_, newF);
    if (!newF) {
      throw Exception(errors::LogicError)
        << "detail_::readFile() failed to return a valid FileBlock object\n";
    }
    return std::unique_ptr<FileBlock>(newF);
  }

  template <typename T>
  void
  Source<T>::closeFile()
  {
    detail_.closeCurrentFile();
    // Cached pointers are no longer valid since the PrincipalCache is
    // cleared after file close.
    cachedRP_ = nullptr;
    cachedSRP_ = nullptr;
  }

  template <typename T>
  std::unique_ptr<RunPrincipal>
  Source<T>::readRun()
  {
    if (!newRP_)
      throw Exception(errors::LogicError)
        << "Error in Source<T>\n"
        << "readRun() called when no RunPrincipal exists\n"
        << "Please report this to the art developers\n";
    cachedRP_ = newRP_.get();
    return std::move(newRP_);
  }

  template <typename T>
  std::unique_ptr<SubRunPrincipal>
  Source<T>::readSubRun(cet::exempt_ptr<RunPrincipal const>)
  {
    if (!newSRP_)
      throw Exception(errors::LogicError)
        << "Error in Source<T>\n"
        << "readSubRun() called when no SubRunPrincipal exists\n"
        << "Please report this to the art developers\n";
    if (subRunIsNew_) {
      if (haveSRLimit_) {
        --remainingSubRuns_;
      }
      subRunIsNew_ = false;
    }
    cachedSRP_ = newSRP_.get();
    return std::move(newSRP_);
  }

  template <typename T>
  std::unique_ptr<EventPrincipal>
  Source<T>::readEvent(cet::exempt_ptr<SubRunPrincipal const>)
  {
    if (haveEventLimit_) {
      --remainingEvents_;
    }
    return std::move(newE_);
  }

  template <typename T>
  void
  Source<T>::finishProductRegistration_(InputSourceDescription& d)
  {
    // These _xERROR_ strings should never appear in branch names; they
    // are here as tracers to help identify any failures in coding.
    ProductDescriptions descriptions;
    h_.registerProducts(
      descriptions,
      ModuleDescription{fhicl::ParameterSet{}.id(), // Dummy
                        "_NAMEERROR_",
                        "_LABELERROR_",
                        ModuleThreadingType::legacy,
                        d.moduleDescription.processConfiguration(),
                        true /*isEmulated*/});
    presentProducts_ = ProductTables{descriptions};
    sourceHelper_.setPresentProducts(cet::make_exempt_ptr(&presentProducts_));
    outputCallbacks_.invoke(presentProducts_);
  }

} // namespace art

#endif /* art_Framework_IO_Sources_Source_h */

// Local Variables:
// mode: c++
// End:
