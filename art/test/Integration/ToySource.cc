#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/SourceHelper.h"
#include "art/Framework/IO/Sources/put_product_in_principal.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/test/Integration/ToySource.h"
#include "art/test/TestObjects/ToyProducts.h"

arttest::ToySource::
ToySource(fhicl::ParameterSet const& ps,
          art::ProductRegistryHelper& helper,
          art::SourceHelper const& sHelper) :
  current_(),
  end_(),
  data_(ps),
  fileData_(),
  sHelper_(sHelper),
  currentFilename_(),
  throw_on_construction_(ps.get<bool>("throw_on_construction", false)),
  throw_on_closeCurrentFile_(ps.get<bool>("throw_on_closeCurrentFile", false)),
  throw_on_readNext_(ps.get<bool>("throw_on_readNext", false)),
  throw_on_readFile_(ps.get<bool>("throw_on_construction", false)),
  vtl_(helper.reconstitutes<std::vector<double>, art::InEvent>("m3"))
{
  if (throw_on_construction_) throw_exception_from("ToySource");
  helper.reconstitutes<int, art::InEvent>("m1");
  helper.reconstitutes<double, art::InSubRun>("s1");
  helper.reconstitutes<double, art::InRun>("r1");
  helper.reconstitutes<bool, art::InEvent>("m2", "a");
  helper.reconstitutes<bool, art::InEvent>("m2", "b");
  helper.reconstitutes<art::Ptr<double>, art::InEvent>("m3");
}

void
arttest::ToySource::closeCurrentFile()
{
  if (throw_on_closeCurrentFile_) throw_exception_from("closeCurrentFile");
  fileData_.clear();
  current_ = iter();
  end_ = iter();
}

bool
arttest::ToySource::readNext(art::RunPrincipal* const& inR,
                             art::SubRunPrincipal* const& inSR,
                             art::RunPrincipal*& outR,
                             art::SubRunPrincipal*& outSR,
                             art::EventPrincipal*& outE)
{
  using art::put_product_in_principal;
  using std::make_unique;

  if (throw_on_readNext_) throw_exception_from("readNext");
  // Have we any more to read?
  if (current_ == end_) return false;

  if (data_.get<bool>("returnTrueWithoutNewFault", false)) return true;

  bool readSomething = false;

  if ((*current_)[0] != -1) // New run
    {
      art::Timestamp runstart; // current time?
      outR = sHelper_.makeRunPrincipal((*current_)[0],  // run number
                                       runstart);     // starting time
      put_product_in_principal(make_unique<double>(76.5), *outR, "r1");
      readSomething = true;
    }
  if ((*current_)[1] != -1) // New subrun
    {
      assert(outR || inR); // Must have one or the other.
      art::SubRunID newSRID;
      if (data_.get<bool>("newRunSameSubRunFault", false) && inSR)
        {
          newSRID = art::SubRunID{inSR->id()};
        }
      else
        {
          newSRID = art::SubRunID(outR?outR->run():inR->run(), (*current_)[1]);
        }
      art::Timestamp runstart; // current time?
      outSR = sHelper_.makeSubRunPrincipal(newSRID.run(),
                                           newSRID.subRun(),
                                           runstart);     // starting time
      put_product_in_principal(make_unique<double>(7.0), *outSR, "s1");
      readSomething = true;
    }
  if ((*current_)[2] != -1)
    {
      assert(outSR || inSR);
      art::Timestamp runstart; // current time?
      outE = sHelper_.makeEventPrincipal(outR?outR->run():inR->run(),
                                         outSR?outSR->subRun():inSR->subRun(),
                                         (*current_)[2],  // event number
                                         runstart);     // starting time
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

      readSomething = true;
    }
  if (readSomething)
    {
      if (data_.get<bool>("returnFalseWithNewFault", false))
        return false;
    }
  else
    {
      throw art::Exception(art::errors::DataCorruption)
        << "Did not read expected info from 'file.'\n";
    }
  ++current_;
  return true;
}

void
arttest::ToySource::throw_exception_from(const char* funcname)
{
  throw art::Exception(art::errors::OtherArt)
    << "Expected exception from DETAIL::"
    << funcname
    << '\n';
}
