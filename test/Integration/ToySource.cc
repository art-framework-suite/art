#include "test/Integration/ToySource.h"

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/put_product_in_principal.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"

arttest::ToySource::ToySource(fhicl::ParameterSet const& ps,
                          art::ProductRegistryHelper& helper,
                          art::PrincipalMaker const &pm) :
  current_(),
  end_(),
  data_(ps),
  fileData_(),
  pm_(pm),
  currentFilename_(),
  throw_on_construction_(ps.get<bool>("throw_on_construction", false)),
  throw_on_closeCurrentFile_(ps.get<bool>("throw_on_closeCurrentFile", false)),
  throw_on_readNext_(ps.get<bool>("throw_on_readNext", false)),
  throw_on_readFile_(ps.get<bool>("throw_on_construction", false))
{
  if (throw_on_construction_) throw_exception_from("ToySource");
  helper.reconstitutes<int, art::InEvent>("m1");
  helper.reconstitutes<double, art::InSubRun>("s1");
  helper.reconstitutes<double, art::InRun>("r1");
  helper.reconstitutes<bool, art::InEvent>("m2", "a");
  helper.reconstitutes<bool, art::InEvent>("m2", "b");
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
  if (throw_on_readNext_) throw_exception_from("readNext");
  // Have we any more to read?
  if (current_ == end_) return false;

  if (data_.get<bool>("returnTrueWithoutNewFault", false)) return true;

  bool readSomething = false;

  if ((*current_)[0] != -1) // New run
  {
    art::Timestamp runstart; // current time?
    outR = pm_.makeRunPrincipal((*current_)[0],  // run number
                                runstart);     // starting time
    put_product_in_principal(std::unique_ptr<double>(new double(76.5)),
                             *outR,
                             "r1");
    readSomething = true;
  }
  if ((*current_)[1] != -1) // New subrun
  {
    assert(outR || inR); // Must have one or the other.
    art::SubRunID newSRID;
    if (data_.get<bool>("newRunSameSubRunFault", false) && inSR)
    {
      newSRID = art::SubRunID(inSR->id());
    }
    else
    {
      newSRID = art::SubRunID(outR?outR->run():inR->run(),
                              (*current_)[1]);
    }
    art::Timestamp runstart; // current time?
    outSR = pm_.makeSubRunPrincipal(newSRID.run(),
                                    newSRID.subRun(),
                                    runstart);     // starting time
    put_product_in_principal(std::unique_ptr<double>(new double(7.0)),
                             *outSR,
                             "s1");
    readSomething = true;
  }
  if ((*current_)[2] != -1)
  {
    assert(outSR || inSR);
    art::Timestamp runstart; // current time?
    outE = pm_.makeEventPrincipal(outR?outR->run():inR->run(),
                                  outSR?outSR->subRun():inSR->subRun(),
                                  (*current_)[2],  // event number
                                  runstart);     // starting time
    art::put_product_in_principal(std::unique_ptr<int>(new int(26)),
                                  *outE,
                                  "m1");
    art::put_product_in_principal(std::unique_ptr<bool>(new bool(false)),
                                  *outE,
                                  "m2",
                                  "a");
    art::put_product_in_principal(std::unique_ptr<bool>(new bool(true)),
                                  *outE,
                                  "m2",
                                  "b");
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

