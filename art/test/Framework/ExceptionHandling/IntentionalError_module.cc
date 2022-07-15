#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/Event.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/types/Atom.h"

namespace art::test {

  class IntentionalError : public EDProducer {
    unsigned const throwAtEventNo_;
    struct Config {
      fhicl::Atom<unsigned> throwAtEventNo{
        fhicl::Name{"throwAtEventNo"},
        fhicl::Comment{"Event number at which to throw an exception."}};
    };

  public:
    using Parameters = Table<Config>;
    IntentionalError(Parameters const& p)
      : EDProducer{p}, throwAtEventNo_{p().throwAtEventNo()}
    {}

  private:
    void
    produce(Event& e) override
    {
      if (e.event() == throwAtEventNo_) {
        throw cet::exception("IntentionalError")
          << "Checking exception-handling behaviors.\n";
      }
    }
  };

} // namespace art::test

DEFINE_ART_MODULE(art::test::IntentionalError)
