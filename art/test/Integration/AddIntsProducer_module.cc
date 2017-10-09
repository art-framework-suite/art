#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Utilities/ToolConfigTable.h"
#include "art/Utilities/make_tool.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "fhiclcpp/types/DelegatedParameter.h"

using namespace fhicl;

namespace {

  struct Config {
    DelegatedParameter tool_table{Name("addIntsImpl")};
    Sequence<std::string> labels{Name("labels")};
  };

  class AddIntsProducer : public art::EDProducer {
  public:
    using Parameters = art::EDProducer::Table<Config>;

    explicit AddIntsProducer(Parameters const& p)
      : labels_{p().labels()}
      , addInts_{art::make_tool<void(int&, int)>(
          p().tool_table.get<fhicl::ParameterSet>(),
          "addInts")}
    {
      produces<arttest::IntProduct>();
    }

    void produce(art::Event& e) override;

  private:
    std::vector<std::string> labels_;
    // Test tool invocation from within module.
    std::function<void(int&, int)> addInts_;
  };

  void
  AddIntsProducer::produce(art::Event& e)
  {
    int value{};
    for (auto const& label : labels_) {
      auto const newVal = e.getValidHandle<arttest::IntProduct>(label)->value;
      addInts_(value, newVal);
    }
    e.put(std::make_unique<arttest::IntProduct>(value));
  }
}

DEFINE_ART_MODULE(AddIntsProducer)
