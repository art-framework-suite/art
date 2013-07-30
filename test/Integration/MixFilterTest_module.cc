#include "boost/test/included/unit_test.hpp"

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/PtrRemapper.h"
#include "art/Framework/IO/ProductMix/MixHelper.h"
#include "art/Framework/Modules/MixFilter.h"
#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Common/CollectionUtilities.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Utilities/InputTag.h"
#include "cetlib/map_vector.h"
#include "cpp0x/memory"
#include "test/TestObjects/ProductWithPtrs.h"
#include "test/TestObjects/ToyProducts.h"

#include "boost/noncopyable.hpp"

namespace arttest {
  class MixFilterTestDetail;
#if ART_TEST_EVENTS_TO_SKIP_CONST
#  define ART_MFT MixFilterTestETSc
#  define ART_TEST_EVENTS_TO_SKIP_CONST_TXT const
#elif defined ART_TEST_EVENTS_TO_SKIP_CONST
#  define ART_MFT MixFilterTestETS
#  define ART_TEST_EVENTS_TO_SKIP_CONST_TXT
#elif defined ART_TEST_OLD_STARTEVENT
#  define ART_MT MixFilterTestOldStartEvent
#elif defined ART_TEST_NO_STARTEVENT
#  define ART_MT MixFilterTestNoStartEvent
#else
// Normal case
#  define ART_MFT MixFilterTest
#endif
  typedef art::MixFilter<MixFilterTestDetail> ART_MFT;
}

class arttest::MixFilterTestDetail : private boost::noncopyable {
public:
  typedef cet::map_vector<unsigned int> mv_t;
  typedef typename mv_t::value_type mvv_t;
  typedef typename mv_t::mapped_type mvm_t;

  // Constructor is responsible for registering mix operations with
  // MixHelper::declareMixOp() and bookkeeping products with
  // MixHelperproduces().
  MixFilterTestDetail(fhicl::ParameterSet const & p,
                      art::MixHelper & helper);

#ifdef ART_TEST_OLD_STARTEVENT
  // Old startEvent signature -- check it still works
  void startEvent();
#elif ! defined ART_TEST_NO_STARTEVENT
  // Optional startEvent(Event const &): initialize state for each event,
  void startEvent(art::Event const &);
#endif

  // Return the number of secondaries to read this time. Declare const
  // if you don't plan to change your class' state.
  size_t nSecondaries() const;

  // Optional eventsToSkip(): number of events to skip at the start of
  // the file.
#ifdef ART_TEST_EVENTS_TO_SKIP_CONST
  size_t eventsToSkip() ART_TEST_EVENTS_TO_SKIP_CONST_TXT { return 7; }
#endif

  // Optional processEventIDs(): after the generation of the event
  // sequence, this function will be called if it exists to provide the
  // sequence of EventIDs.
  void processEventIDs(art::EventIDSequence const & seq);

  // Optional.finalizeEvent(): (eg) put bookkeping products in event. Do
  // *not* place mix products into the event: this will already have
  // been done for you.
  void finalizeEvent(art::Event & t);

  // Mixing functions. Note that they do not *have* to be member
  // functions of this detail class: they may be member functions of a
  // completely unrelated class; free functions or function objects
  // provided they (or the function object's operator()) have the
  // expected signature.
  template <typename T>
  bool
  mixByAddition(std::vector<T const *> const &,
                T &,
                art::PtrRemapper const &);

  bool
  aggregateDoubleCollection(std::vector<std::vector<double> const *> const & in,
                            std::vector<double> &out,
                            art::PtrRemapper const &);

  bool
  aggregate_map_vector(std::vector<mv_t const *> const & in,
                       mv_t & out,
                       art::PtrRemapper const &);

  bool
  mixPtrs(std::vector<std::vector<art::Ptr<double> > const *> const & in,
          std::vector<art::Ptr<double> > &out,
          art::PtrRemapper const & remap);

#ifndef ART_NO_MIX_PTRVECTOR
  bool
  mixPtrVectors(std::vector<art::PtrVector<double> const *> const & in,
                art::PtrVector<double> &out,
                art::PtrRemapper const & remap);
#endif

  bool
  mixProductWithPtrs(std::vector<arttest::ProductWithPtrs const *> const & in,
                     arttest::ProductWithPtrs & out,
                     art::PtrRemapper const & remap);

  bool
  mixmap_vectorPtrs(std::vector<std::vector<art::Ptr<cet::map_vector<unsigned int>::value_type> > const *> const & in,
                    std::vector<art::Ptr<cet::map_vector<unsigned int>::value_type> > &out,
                    art::PtrRemapper const & remap);

  template <typename COLL>
  void verifyInSize(COLL const & in) const;

private:
  size_t nSecondaries_;
  bool testRemapper_;
  std::vector<size_t> doubleVectorOffsets_, map_vectorOffsets_;
  std::unique_ptr<art::EventIDSequence> eIDs_;
  bool startEvent_called_;
  bool processEventIDs_called_;
  int currentEvent_;
  bool testZeroSecondaries_;
  bool testPtrFailure_;
};

template <typename COLL>
inline
void
arttest::MixFilterTestDetail::
verifyInSize(COLL const & in) const
{
  BOOST_REQUIRE_EQUAL(in.size(), (currentEvent_ == 2 && testZeroSecondaries_) ? 0 : nSecondaries_);
}


arttest::MixFilterTestDetail::
MixFilterTestDetail(fhicl::ParameterSet const & p,
                    art::MixHelper & helper)
  :
  nSecondaries_(p.get<size_t>("numSecondaries", 1)),
  testRemapper_(p.get<bool>("testRemapper", true)),
  doubleVectorOffsets_(),
  map_vectorOffsets_(),
  eIDs_(),
  startEvent_called_(false),
  processEventIDs_called_(false),
  currentEvent_(-1),
  testZeroSecondaries_(p.get<bool>("testZeroSecondaries", false)),
  testPtrFailure_(p.get<bool>("testPtrFailure", false))
{
  std::string mixProducerLabel(p.get<std::string>("mixProducerLabel",
                               "mixProducer"));
  helper.produces<std::string>(); // "Bookkeeping"
  helper.produces<art::EventIDSequence>(); // "Bookkeeping"
  helper.declareMixOp
  (art::InputTag(mixProducerLabel, "doubleLabel"),
   &MixFilterTestDetail::mixByAddition<double>, *this);
  helper.declareMixOp
  (art::InputTag(mixProducerLabel, "IntProductLabel"),
   &MixFilterTestDetail::mixByAddition<arttest::IntProduct>, *this);
  helper.declareMixOp
  (art::InputTag(mixProducerLabel, "stringLabel", "SWRITE"),
   &MixFilterTestDetail::mixByAddition<std::string>, *this);
  helper.declareMixOp
  (art::InputTag(mixProducerLabel, "doubleCollectionLabel"),
   &MixFilterTestDetail::aggregateDoubleCollection, *this);
  helper.declareMixOp
  (art::InputTag(mixProducerLabel, "doubleVectorPtrLabel"),
   &MixFilterTestDetail::mixPtrs, *this);
#ifndef ART_NO_MIX_PTRVECTOR
  helper.declareMixOp
  (art::InputTag(mixProducerLabel, "doublePtrVectorLabel"),
   &MixFilterTestDetail::mixPtrVectors, *this);
#endif
  helper.declareMixOp
  (art::InputTag(mixProducerLabel, "ProductWithPtrsLabel"),
   &MixFilterTestDetail::mixProductWithPtrs, *this);
  helper.declareMixOp
  (art::InputTag(mixProducerLabel, "mapVectorLabel"),
   &MixFilterTestDetail::aggregate_map_vector, *this);
  helper.declareMixOp
  (art::InputTag(mixProducerLabel, "intVectorPtrLabel"),
   &MixFilterTestDetail::mixmap_vectorPtrs, *this);
}

#ifndef ART_TEST_NO_STARTEVENT
void
arttest::MixFilterTestDetail::
#  ifdef ART_TEST_OLD_STARTEVENT
startEvent()
#  else
// Normal case
startEvent(art::Event const &)
#  endif
{
  startEvent_called_ = true;
  eIDs_.reset();
  ++currentEvent_;
}
#endif

size_t
arttest::MixFilterTestDetail::
nSecondaries() const
{
  return (currentEvent_ == 2 && testZeroSecondaries_) ? 0 : nSecondaries_;
}

void
arttest::MixFilterTestDetail::
processEventIDs(art::EventIDSequence const & seq)
{
#ifdef ART_TEST_NO_STARTEVENT
// Need to deal with this here
  ++currentEvent_;
#endif
  processEventIDs_called_ = true;
  eIDs_.reset(new art::EventIDSequence(seq));
}

void
arttest::MixFilterTestDetail::
finalizeEvent(art::Event & e)
{
  e.put(std::unique_ptr<std::string>(new std::string("BlahBlahBlah")));
  e.put(std::move(eIDs_));
#ifndef ART_TEST_NO_STARTEVENT
  BOOST_REQUIRE(startEvent_called_);
  startEvent_called_ = false;
#endif
  BOOST_REQUIRE(processEventIDs_called_);
  processEventIDs_called_ = false;
}

template<typename T>
bool
arttest::MixFilterTestDetail::
mixByAddition(std::vector<T const *> const & in,
              T & out,
              art::PtrRemapper const &)
{
  verifyInSize(in);
  for (typename std::vector<T const *>::const_iterator
       i = in.begin(),
       e = in.end();
       i != e;
       ++i) {
    out += **i;
  }
  return true; //  Always want product in event.
}

bool
arttest::MixFilterTestDetail::
aggregateDoubleCollection(std::vector<std::vector<double> const *> const & in,
                          std::vector<double> &out,
                          art::PtrRemapper const &)
{
  verifyInSize(in);
  art::flattenCollections(in, out, doubleVectorOffsets_);
  return true; //  Always want product in event.
}

bool
arttest::MixFilterTestDetail::
aggregate_map_vector(std::vector<mv_t const *> const & in,
                     mv_t & out,
                     art::PtrRemapper const &)
{
  verifyInSize(in);
  art::flattenCollections(in, out, map_vectorOffsets_);
  return true; //  Always want product in event.
}

bool
arttest::MixFilterTestDetail::
mixPtrs(std::vector<std::vector<art::Ptr<double> > const *> const & in,
        std::vector<art::Ptr<double> > &out,
        art::PtrRemapper const & remap)
{
  verifyInSize(in);
  remap(in,
        std::back_inserter(out),
        doubleVectorOffsets_);
  if (testPtrFailure_) {
    BOOST_REQUIRE_THROW(*out.front(), art::Exception);
  }
  return true; //  Always want product in event.
}

#ifndef ART_NO_MIX_PTRVECTOR
bool
arttest::MixFilterTestDetail::
mixPtrVectors(std::vector<art::PtrVector<double> const *> const & in,
              art::PtrVector<double> &out,
              art::PtrRemapper const & remap)
{
  verifyInSize(in);
  remap(in,
        std::back_inserter(out),
        doubleVectorOffsets_);
  return true; //  Always want product in event.
}
#endif

bool
arttest::MixFilterTestDetail::
mixProductWithPtrs(std::vector<arttest::ProductWithPtrs const *> const & in,
                   arttest::ProductWithPtrs & out,
                   art::PtrRemapper const & remap)
{
  verifyInSize(in);
#ifndef ART_NO_MIX_PTRVECTOR
  remap(in,
        std::back_inserter(out.ptrVectorDouble()),
        doubleVectorOffsets_,
        &arttest::ProductWithPtrs::ptrVectorDouble);
#endif
  remap(in,
        std::back_inserter(out.vectorPtrDouble()),
        doubleVectorOffsets_,
        &arttest::ProductWithPtrs::vectorPtrDouble);
  // Throw-away object to test non-standard remap interface.
  arttest::ProductWithPtrs tmp;
  remap(in,
        std::back_inserter(tmp.vectorPtrDouble()),
        doubleVectorOffsets_,
        &arttest::ProductWithPtrs::vpd_);
  return true; //  Always want product in event.
}

bool
arttest::MixFilterTestDetail::
mixmap_vectorPtrs(std::vector<std::vector<art::Ptr<cet::map_vector<unsigned int>::value_type> > const *> const & in,
                  std::vector<art::Ptr<cet::map_vector<unsigned int>::value_type> > &out,
                  art::PtrRemapper const & remap)
{
  verifyInSize(in);
  remap(in,
        std::back_inserter(out),
        map_vectorOffsets_);
  return true; //  Always want product in event.
}

namespace arttest {
  DEFINE_ART_MODULE(ART_MFT)
}

