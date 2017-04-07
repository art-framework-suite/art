// Our simulated input file format is:
// A parameter in a parameter set, which contains a vector of vector of int.
// Each inner vector is a triplet of run/subrun/event number.
//   -1 means no new item of that type
//

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/Source.h"
#include "art/Framework/IO/Sources/SourceHelper.h"
#include "art/Framework/IO/Sources/put_product_in_principal.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>
#include <vector>

namespace art {
  namespace test {
    class SimpleSource;
  }
}

namespace art {
  // We don't want the file services: we must say so by specializing the
  // template *before* specifying the typedef.
  template<>
  struct Source_wantFileServices<art::test::SimpleSource> {
    static constexpr bool value {false};
  };
}

class art::test::SimpleSource {
public:

  SimpleSource(fhicl::ParameterSet const& ps,
               art::ProductRegistryHelper& help,
               art::SourceHelper const& sHelper);

  void closeCurrentFile();

  void readFile(std::string const& name,
                FileBlock*& fb);

  bool readNext(RunPrincipal const* const inR,
                SubRunPrincipal const* const inSR,
                RunPrincipal*& outR,
                SubRunPrincipal*& outSR,
                EventPrincipal*& outE);

protected:

  using eventNumbers_t = std::vector<std::tuple<int,int,int>>;
  using iter = eventNumbers_t::const_iterator;

  iter current_ {};
  iter end_ {};
  fhicl::ParameterSet data_;
  eventNumbers_t fileData_ {};

  art::SourceHelper const& sHelper_;
  std::string currentFilename_ {};
  RunID currentRunID_ {};
  SubRunID currentSubRunID_ {};
  EventID currentEventID_ {};

  art::TypeLabel vtl_;
};

art::test::SimpleSource::SimpleSource(fhicl::ParameterSet const& ps,
                                      art::ProductRegistryHelper& helper,
                                      art::SourceHelper const& sHelper) :
  data_{ps},
  sHelper_{sHelper},
  vtl_{helper.reconstitutes<std::vector<double>, art::InEvent>("m3")}
{
  helper.reconstitutes<int, art::InEvent>("m1");
  helper.reconstitutes<double, art::InSubRun>("s1");
  helper.reconstitutes<double, art::InRun>("r1");
  helper.reconstitutes<bool, art::InEvent>("m2", "a");
  helper.reconstitutes<bool, art::InEvent>("m2", "b");
  helper.reconstitutes<art::Ptr<double>, art::InEvent>("m3");
}

void
art::test::SimpleSource::closeCurrentFile()
{
  fileData_.clear();
  current_ = iter{};
  end_ = iter{};
}

void
art::test::SimpleSource::readFile(std::string const& name, art::FileBlock*& fb)
{
  if (!data_.get_if_present(name, fileData_)) {
    throw art::Exception(art::errors::Configuration)
      << "ToyReader expects to find a parameter representing a file's\n"
      << "contents whose name is "
      << name
      << "\n";
  }
  currentFilename_ = name;
  current_ = fileData_.begin();
  end_ = fileData_.end();
  fb = new art::FileBlock{art::FileFormatVersion{1, "SimpleSource 2017"}, currentFilename_};
}


bool
art::test::SimpleSource::readNext(RunPrincipal const* const,
                                  SubRunPrincipal const* const,
                                  RunPrincipal*& outR,
                                  SubRunPrincipal*& outSR,
                                  EventPrincipal*& outE)
{
  using art::put_product_in_principal;
  using std::make_unique;

  // Have we any more to read?
  if (current_ == end_) return false;

  int r, sr, e;
  std::tie(r, sr, e) = *current_;
  ++current_;

  if (r != -1) // New run
    {
      currentRunID_ = RunID{RunNumber_t(r)};
      outR = sHelper_.makeRunPrincipal(currentRunID_, art::Timestamp{});
      put_product_in_principal(make_unique<double>(76.5), *outR, "r1");
    }
  if (sr != -1) // New subrun
    {
      currentSubRunID_ = SubRunID{currentRunID_, SubRunNumber_t(sr)};
      outSR = sHelper_.makeSubRunPrincipal(currentSubRunID_, art::Timestamp{});
      put_product_in_principal(make_unique<double>(7.0), *outSR, "s1");
    }
  if (e != -1) // New event
    {
      currentEventID_ = EventID{currentSubRunID_, EventNumber_t(e)};
      outE = sHelper_.makeEventPrincipal(currentEventID_, art::Timestamp{});
      put_product_in_principal(make_unique<int>(26), *outE, "m1");
      put_product_in_principal(make_unique<bool>(false), *outE, "m2", "a");
      put_product_in_principal(make_unique<bool>(true), *outE, "m2", "b");
      {
        std::vector<double> nums {1.0, 3.7, 5.2};
        put_product_in_principal(make_unique<decltype(nums)>(std::move(nums)), *outE, "m3");
      }
      put_product_in_principal(make_unique<art::Ptr<double>>(sHelper_.makePtr<double>(vtl_, *outE, 1)),
                               *outE,
                               "m3");

    }
  return true;
}

DEFINE_ART_INPUT_SOURCE(art::Source<art::test::SimpleSource>)
