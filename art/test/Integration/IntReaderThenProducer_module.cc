#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/types/Atom.h"

#include <memory>

namespace art {
  namespace test {
    class IntReaderThenProducer;
  }
}

using namespace fhicl;

class art::test::IntReaderThenProducer : public SharedProducer {
public:
  struct Config {
    Atom<std::string> inputTag{Name{"inputTag"}};
    Atom<int> deltaValue{Name{"deltaValue"}};
    Atom<bool> shouldSucceed{
      Name{"shouldSucceed"},
      Comment{
        "The 'shouldSucceed' parameter indicates if the product retrieval,\n"
        "corresponding to the above 'inputTag' value, should be successful."},
      true};
  };
  using Parameters = Table<Config>;
  explicit IntReaderThenProducer(Parameters const& p);

  // Plugins should not be copied or assigned.
  IntReaderThenProducer(IntReaderThenProducer const&) = delete;
  IntReaderThenProducer(IntReaderThenProducer&&) = delete;
  IntReaderThenProducer& operator=(IntReaderThenProducer const&) = delete;
  IntReaderThenProducer& operator=(IntReaderThenProducer&&) = delete;

private:
  void produce(Event& e, Services const&) override;

  ProductToken<arttest::IntProduct> const token_;
  int const deltaValue_;
  bool const shouldSucceed_;
};

art::test::IntReaderThenProducer::IntReaderThenProducer(Parameters const& p)
  : SharedProducer{p}
  , token_{consumes<arttest::IntProduct>(p().inputTag())}
  , deltaValue_{p().deltaValue()}
  , shouldSucceed_{p().shouldSucceed()}
{
  produces<arttest::IntProduct>();
  async<InEvent>();
}

void
art::test::IntReaderThenProducer::produce(Event& e, Services const&)
{
  // getByToken adds parent for the about-to-be-created IntProduct.
  Handle<arttest::IntProduct> intH;
  bool const fetched{e.getByToken(token_, intH)};
  assert(shouldSucceed_ == fetched);
  auto new_value = std::make_unique<arttest::IntProduct>(deltaValue_);
  if (shouldSucceed_) {
    new_value->value += intH->value;
  }
  e.put(move(new_value));
}

DEFINE_ART_MODULE(art::test::IntReaderThenProducer)
